#include "Storage.h"
#include <string.h>
#include <stdlib.h>

#define INIT_CAPACITY 8
#define CAPACITY_K 2
#define ID_MAX UINT32_MAX
#define ID_MIN 1ull
//кринжовое хранилище (стоило назвать Collection или DB)
//и надо было писать эту задачу на PostgreSQL

typedef struct Node{
    StEntry entry;
    struct Node* left;
    struct Node* right;
    struct Node* parent;
    int height;
} Node;

typedef struct Storage{
    ID_TYPE size;
    ID_TYPE curId;
    size_t stsz;
    Node* root;
} Storage;

//итератор
typedef struct StIterator{
    const Storage* storage;
    int8_t errCode; //1 - corrupted, 2 - end, 3 - before start
    ID_TYPE pos;
    Node* node;
} StIterator;

static inline int max_int(int a, int b) { 
    return (a > b) ? a : b; 
}

static inline int height(Node* N) { 
    return N ? N->height : 0; 
}

static inline int getBalance(Node* N) { 
    return N ? height(N->left) - height(N->right) : 0; 
}

Node* minimum(Node* node) {
    if (!node) return NULL;
    while (node->left) {
        node = node->left;
    }
    return node;
}

Node* maximum(Node* node)  {
    if (!node) return NULL;
    while (node->right) {
        node = node->right;
    }
    return node;
}

static Node* successor(Node* node) {
    if (!node) return NULL;
    if (node->right) return minimum(node->right);
    Node* p = node->parent;
    while (p && node == p->right) {
        node = p;
        p = p->parent;
    }
    return p;
}

static Node* predecessor(Node* node) {
    if (!node) return NULL;
    if (node->left) return maximum(node->left);
    Node* p = node->parent;
    while (p && node == p->left) {
        node = p;
        p = p->parent;
    }
    return p;
}

static void freeTree(Node* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node->entry.var);
    free(node);
}

static void updateHeight(Node *node) {
    int lh = height(node->left);
    int rh = height(node->right);
    // Высота узла равна высоте более высокого поддерева + 1
    if (lh > rh) {
        node->height = lh + 1;
    } else {
        node->height = rh + 1;
    }
}

static Node* rotateRight(Node* node){
    Node* child  = node->left;
    Node* grandc = child->right;
    
    child->right = node;
    node->left   = grandc;
    
    node->parent = child;
    if(grandc != NULL)
        grandc->parent = node;

    updateHeight(node);
    updateHeight(child);
    
    return child;
}

static Node* rotateLeft(Node* node){
    Node* child  = node->right;
    Node* grandc = child->left;
    
    child->left = node;
    node->right = grandc;

    node->parent = child;
    if(grandc != NULL)
        grandc->parent = node;

    updateHeight(node);
    updateHeight(child);
    
    return child;
}

static Node* rotate(Node* node){
    int bl = getBalance(node); 
    //левосторонний перекос
    if(bl > 1){
        Node* child = node->left;
        int blc = getBalance(child);
        if(blc >= 0){
            return rotateRight(node);
        }else{
            node->left = rotateLeft(child);
            return rotateRight(node);
        }
    }else if(bl < -1){//правосторонний перекос
        Node* child = node->right;
        int blc = getBalance(child);
        if(blc <= 0){
            return rotateLeft(node);
        }else{
            node->right = rotateRight(child);
            return rotateLeft(node);
        }
    }
    return node;
}

/// @brief Установка итератора в начало. Возвращает итератор
/// @param st целевое хранилище
/// @return итератор
StIterator* stbeg (const Storage* const st){
    if(!st)
        return NULL;

    StIterator* it = malloc(sizeof(StIterator));
    if(!it)
        return NULL;
    it->storage = st;
    it->node = minimum(st->root);
    it->pos = 0;
    it->errCode = (it->node == NULL) ? 1 : 0;
    return it;
}
/// @brief Установка итератора в конец. Возвращает итератор
/// @param st целевое хранилище
/// @return итератор
StIterator* stend (const Storage* const st){
    if(!st)
        return NULL;

    StIterator* it = malloc(sizeof(StIterator));
    if(!it)
        return NULL;

    it->storage = st;
    it->node = NULL;
    it->pos = st->size;
    it->errCode = (st->root) == NULL ? 1 : 2;
    return it;
}
/// @brief Устанавливает итератор на следующую позицию
/// @param it Указатель на итератор
void stnext(StIterator* it){
    if(it->errCode == 2 || it->errCode == 1){
        it->errCode = 1;
        it->node = NULL;
        it->pos++;
        return;
    }
    if(it->errCode == 3){
        it->errCode = 0;
        it->node = minimum(it->storage->root);
        it->pos = 0;
        return;
    }

    it->node = successor(it->node);
    it->pos++;
    if (it->node == NULL) {
        it->errCode = 2;
    }
    

}
/// @brief Устанавливает итератор на предыдущую позицию
/// @param it Указатель на итератор
void stprev(StIterator* it){
    if(it->errCode == 1 || it->errCode == 3){
        it->errCode = 1;
        it->node = NULL;
        it->pos = 0;
        return;
    }
    
    if(it->errCode == 2){
        it->errCode = 0;
        it->pos = it->storage->size - 1;
        it->node = maximum(it->storage->root);
        return;
    }
    it->node = predecessor(it->node);
    it->pos--;
    if (it->node == NULL) {
        it->errCode = 3;
    }
}
/// @brief Разыменовывание итератора
/// @param it Указатель на итератор
/// @return запись в коллекции вместе с id
StEntry* stitgt(const StIterator* const it){
    if (!it || it->errCode != 0) return NULL;
    return &it->node->entry;
}
/// @brief удаление итератора
/// @param it указатель на итератор
void        rmiter(StIterator* it){
    if(it)
        free(it);
}

int8_t itcomp(const StIterator* const it1, const StIterator* const it2){
    if(it1->storage != it2->storage)
        return ST_ARG_ERR;
    if(it1->node == it2->node)
        return 0;
    if(!it1 || !it2)
        return ST_ARG_ERR;
    return it1->pos < it2->pos ? -1 : (it1->pos == it2->pos ? 0 : 1); 
}

int8_t gtitid(StIterator* it, const uint32_t id){
    StIterator* end = stend(it->storage);
    StEntry* entry;
    for(; itcomp(it, end) != 0; stnext(it)){
        entry = stitgt(it);
        if(entry->id == id){
            rmiter(end);
            return 0;
        }
    }
    rmiter(end);
    return ST_NO_SUCH_ID;
}




/// @brief  создание хранилища
/// @param structsz размер хранимой структуры
/// @return возвращает указатель на созданное хранилище
Storage* mkstor (const size_t structsz){
    Storage* storage = (Storage*) malloc(sizeof(Storage));
    if(!storage)
        return NULL;

    storage->stsz  = structsz;
    storage->size  = 0;
    storage->curId = 1;

    storage->root = NULL;
    return storage;
}
/// @brief удаление хранилища
/// @param st указатель на удалаяемое хранилище
void     remstor(Storage* st){
    if(st){
        freeTree(st->root);
        free(st);
    }
}
/// @brief 
/// @param storage указатель на хранилище 
/// @return количиство элементов в коллекции
ID_TYPE stsize(const Storage* const storage){
    return storage ? storage->size : 0;
}

/// @brief Пересчёт текущего id для добавления. новый curId не может быть меньше текущего
/// функция ищет ближайший свободный id справа
/// @param storage хранилище, у которого будет пересчёт id
void recid(Storage* storage){
    if(!storage) return;

    while (stgt(storage, storage->curId) != NULL) {
        storage->curId++;
    }
}

static Node* createNode(Node* parent, const ID_TYPE id, const void* const var){
    Node*  node = malloc(sizeof(Node));
    node->entry.id = id;
    node->entry.var = var;
    node->height = 0;
    node->left = NULL;
    node->right = NULL;
    node->parent = parent;
    return node;
}

static Node* balanceUp(Storage* st, Node* curr) {
    
    Node* parent;
    Node* new_root = NULL;

    while (curr != NULL) {
        
        parent = curr->parent;
        updateHeight(curr);
        new_root = rotate(curr);

        if (new_root != curr) {
            new_root->parent = parent;
            if (parent == NULL) {
                st->root = new_root;
            } else if (parent->left == curr) {
                parent->left = new_root;
            } else {
                parent->right = new_root;
            }
        }
        curr = parent; 
    }
    return new_root;
}

static Node* insert(Storage* st, const ID_TYPE id, const void* const var){
    
    if(st->root == NULL){
        Node* node = createNode(NULL, id, var); 
        st->root = node;
        st->size++;
        recid(st);
        return node;
    }

    Node* curr = st->root;
    Node* currparent;
    while(curr != NULL){
        currparent = curr;
        if(id < curr->entry.id){
            curr = curr->left;
        }else if (id > curr->entry.id){
            curr = curr->right;
        }else
            return curr;
    }
    Node* node = createNode(currparent, id, var);
    if (id < currparent->entry.id) {
        currparent->left = node;
    } else {
        currparent->right = node;
    }

    st->size++;
    recid(st);
    return balanceUp(st, node);
}

/// @brief Вставка по id. Insert into st values (id, var).  
/// @param storage Вставка в хранилище по указателю
/// @param id Если id занят, вставка не произойдёт - ошибка ST_NO_SUCH_ID
/// @param var Указатель на вставляемое значение 
/// @return код ошибки
int8_t stin(Storage* storage, const ID_TYPE id, void* var){
    if(!storage || !var) return ST_ARG_ERR;

    if (stgt(storage, id) != NULL) return ST_BUSY_ID;
    Node* node = insert(storage, id, var);
    if(node == NULL)
        return ST_MEM_ALLOC_ERR;

    return ST_NOERR;
}
/// @brief Вставка с автоматическим указанием id. Insert into st values (var).
/// @param storage Вставка в хранилище по указателю
/// @param var Вставляемое значение
/// @return код ошибки
int8_t stil (Storage* storage, void* var){
    return stin(storage, storage->curId, var);
}

static Node* rem(Storage* st, const ID_TYPE id){
    
    if(st->root == NULL){
        return NULL;
    }
    
    Node* curr = st->root;

    while(curr != NULL && curr->entry.id != id){
        if(id < curr->entry.id)
            curr = curr->left;
        else
            curr = curr->right;
    }
    if(curr != NULL){
        
        if(curr->left != NULL && curr->right != NULL){
            Node* succ = successor(curr);
            StEntry temp = curr->entry;
            curr->entry = succ->entry;
            succ->entry = temp;
            
            curr = succ;
        }
        Node* child = (curr->left != NULL) ? curr->left : curr->right;
        Node* parent = curr->parent;

        if (child != NULL) {
            child->parent = parent;
        }

        if (parent == NULL) {
            st->root = child;
        } else if (parent->left == curr) {
            parent->left = child;
        } else {
            parent->right = child;
        }
        balanceUp(st, child);
        free(curr->entry.var);
        free(curr);
        st->size--;
        return child;
    }
    return NULL;
}

/// @brief Удаление по id. Delete from st where st.id = ? 
/// @param storage Удаление из хранилища
/// @param id Идентификатор удаляемой записи
/// @return код ошибки
int8_t strm     (Storage* storage, const ID_TYPE id){
    if(!storage || !storage->root)
        return ST_ARG_ERR;
    
    rem(storage, id);
    return ST_NOERR;
}
/// @brief Удаление по значению. Delete from st where st.var = ? 
/// @param storage Удаление из хранилища
/// @param comp Указатель на функцию компаратора
/// @param var Целевое значение
/// @return код ошибки. последняя цифра значит что случилось, отсальные - это id*10
int16_t ster (Storage* storage, comparator comp, void* var){
    if(!storage || !comp || !var)
        return ST_ARG_ERR;
    ID_TYPE sz = 0;
    ID_TYPE* ind = stfa(storage, comp, var, &sz);//находим все id подходящих записей
    if(ind == NULL)
        return ST_NO_SUCH_ID;
    uint8_t ret = 0;
    for(ID_TYPE i = 0; i < sz; i++){//удаление по id
        ret = strm(storage, ind[i]);
        if(ret != 0){
            free(ind);
            return (i+1)*10 + ret;
        }
    }
    return 0;
}


/// @brief SELECT * FROM st WHERE id = ?
/// @param storage целевое хранилище
/// @param id идентификатор записи
/// @return Запись. Если не найдена - NULL
StEntry* stgt (const Storage* const storage, const ID_TYPE id){
    if(!storage)
        return NULL;
    StIterator* it  = stbeg(storage);
    StIterator* end = stend(storage);
    StEntry* entry;
    for(; itcomp(it, end) != 0; stnext(it)){
        entry = stitgt(it);
        if(entry->id == id){
            rmiter(it);
            rmiter(end);
            return entry;
        }
    }
    rmiter(it);
    rmiter(end);
    return NULL;
}
/// @brief SELECT * FROM st WHERE var = ? LIMIT 1
/// @param storage целевое хранилище
/// @param comp указатель на компаратор
/// @param var целевое значение
/// @return Первая подходящая запись. Не найдена - NULL
ID_TYPE  stff (const Storage* const storage, comparator comp, const void* const var){
    if(!storage || !comp || !var)
        return ID_MAX;

    StIterator* it  = stbeg(storage);
    StIterator* end = stend(storage);

    for(;itcomp(it, end) != 0;stnext(it)){
        if(comp(&it->node->entry.var, var) == 0){
            rmiter(it);
            rmiter(end);
            return it->node->entry.id;
        }
    }
    rmiter(it);
    rmiter(end);
    return ID_MAX;
}
/// @brief SELECT * FROM st WHERE var = ? 
/// @param storage целевое хранилище
/// @param comp указатель на компаратор
/// @param var целевое значение
/// @param retsz размер возращённого массива идентификаторов
/// @return указатель на массив идентификаторв. Ничего не найдено - NULL
ID_TYPE* stfa (const Storage* const storage, comparator comp, const void* const var, ID_TYPE* retsz){
    if(!storage || !comp || !var || !retsz)
        return NULL;
    *retsz = 0;

    ID_TYPE* ids = (ID_TYPE*) malloc(storage->size * sizeof(ID_TYPE));
    if(!ids)
        return NULL;
        
    StIterator* it  = stbeg(storage);
    StIterator* end = stend(storage);

    for(;itcomp(it, end) != 0;stnext(it)){
        if(comp(&it->node->entry.var, var) == 0){
            ids[*retsz] = it->node->entry.id;
            *retsz = *retsz + 1;
        }
    }

    if(*retsz == 0){
        free(ids);
        rmiter(it);
        rmiter(end);
        return NULL;
    }  

    ID_TYPE* newids = (ID_TYPE*)realloc(ids, *retsz * sizeof(ID_TYPE));
    if(!newids){
        rmiter(it);
        rmiter(end);
        return NULL;
    }

    rmiter(it);
    rmiter(end);

    ids = newids;
    return ids;
}


int8_t stchg(Storage* storage, const ID_TYPE id, changer ch, void* var){
    if(!storage || !ch)
        return ST_ARG_ERR;
    StEntry* entry = stgt(storage, id);
    if(!entry)
        return ST_NO_SUCH_ID;
    ch(entry->var, var);
    return ST_NOERR;
}

static void visit_node(Node* node, int depth, int is_left, st_visit_func func) {
    if (!node) return;
    func(node->entry.id, node->entry.var, depth, is_left);
    visit_node(node->left,  depth + 1, 1, func);
    visit_node(node->right, depth + 1, 0, func);
}

void st_visit_tree(Storage* st, st_visit_func func) {
    if (!st || !func) return;
    visit_node(st->root, 0, 0, func);
}