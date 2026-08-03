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
    struct Node* next;
    struct Node* prev;
} Node;

typedef struct Storage{
    ID_TYPE size;
    ID_TYPE curId;
    size_t stsz;
    Node* head;
    Node* tail;
    comparator comp;
} Storage;

//итератор
typedef struct StIterator{
    const Storage* storage;
    int8_t errCode; //1 - corrupted, 2 - end, 3 - before start
    ID_TYPE pos;
    Node* node;
} StIterator;


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
    it->node = st->head;
    it->pos = 0;
    it->errCode = st->head == NULL ? 1 : 0;
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
    it->errCode = st->head == NULL ? 1 : 2;
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
        it->node = it->storage->head;
        it->pos = 0;
        return;
    }
    if(it->node != it->storage->tail){
        it->node = it->node->next;
        it->pos++;
    }
    else{
        it->node = NULL;
        it->errCode = 2;
        it->pos++;
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
        it->node = it->storage->tail;
        return;
    }
    if(it->node != it->storage->head){
        it->node = it->node->prev;
        it->pos--;
    }
    else{
        it->node = NULL;
        it->errCode = 3;
        it->pos = 0;
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
Storage* mkstor (const size_t structsz, comparator comp){
    Storage* storage = (Storage*) malloc(sizeof(Storage));
    if(!storage || !comp)
        return NULL;

    storage->stsz  = structsz;
    storage->size  = 0;
    storage->curId = 1;
    storage->comp  = comp;

    storage->head  = NULL;
    storage->tail  = NULL;
    return storage;
}
/// @brief удаление хранилища
/// @param st указатель на удалаяемое хранилище
void     remstor(Storage* st){
    if(st){
        for(Node* node = st->head; node != NULL; node = st->head) {
            st->head = st->head->next;
            free(node);
        }
        free(st);
    }
}
/// @brief 
/// @param storage указатель на хранилище 
/// @return количиство элементов в коллекции
ID_TYPE stsize(const Storage* const storage){
    return storage->size;
}

/// @brief Пересчёт текущего id для добавления. новый curId не может быть меньше текущего
/// функция ищет ближайший свободный id справа
/// @param storage хранилище, у которого будет пересчёт id
void recid(Storage* storage){
    if(!storage) return;
    while(1) {
        Node* curr = storage->head;
        int busy = 0;
        while(curr != NULL) {
            if(curr->entry.id == storage->curId) {
                busy = 1;
                break;
            }
            curr = curr->next;
        }
        if(!busy) break; // свободный ID
        storage->curId++;
    }
}


/// @brief Вставка по id. Insert into st values (id, var).  
/// @param storage Вставка в хранилище по указателю
/// @param id Если id занят, вставка не произойдёт - ошибка ST_NO_SUCH_ID
/// @param var Указатель на вставляемое значение 
/// @return код ошибки
int8_t stin(Storage* storage, const ID_TYPE id, void* var){
    if(!storage || !var) return ST_ARG_ERR;

    if (stgt(storage, id) != NULL) return ST_BUSY_ID;

    Node* node = malloc(sizeof(Node));
    if(!node) return ST_MEM_ALLOC_ERR;
    
    node->entry.id = id;
    node->entry.var = var;
    node->next = NULL;
    node->prev = NULL;

    if(storage->head == NULL){
        storage->head = node;
        storage->tail = node;
        storage->size++;
        recid(storage);
        return ST_NOERR;
    }

    // прямой проход для сортировки
    Node* curr = storage->head;
    while(curr != NULL) {
        if(storage->comp(var, curr->entry.var) < 0) {
            break; 
        }
        curr = curr->next;
    }

    if(curr != NULL) {
        // вставка до curr
        node->next = curr;
        node->prev = curr->prev;
        if(curr->prev) curr->prev->next = node;
        else storage->head = node;
        curr->prev = node;
    } else {
        //элемент больше всех - в хвост
        node->prev = storage->tail;
        storage->tail->next = node;
        storage->tail = node;
    }

    storage->size++;
    recid(storage);
    
    return ST_NOERR;
}
/// @brief Вставка с автоматическим указанием id. Insert into st values (var).
/// @param storage Вставка в хранилище по указателю
/// @param var Вставляемое значение
/// @return код ошибки
int8_t stil (Storage* storage, void* var){
    return stin(storage, storage->curId, var);
}


/// @brief Удаление по id. Delete from st where st.id = ? 
/// @param storage Удаление из хранилища
/// @param id Идентификатор удаляемой записи
/// @return код ошибки
int8_t strm     (Storage* storage, const ID_TYPE id){
    if(!storage || !storage->head)
        return ST_ARG_ERR;
    StIterator* it = stbeg(storage);
    int8_t err = gtitid(it, id);
    
    if(err != 0){
        rmiter(it);
        return ST_NO_SUCH_ID;
    }
    Node* delnode = it->node;
    if(delnode == storage->head)
        storage->head = delnode->next;
    else
        delnode->prev->next = delnode->next;

    if(delnode == storage->tail)
        storage->tail = delnode->prev;
    else
        delnode->next->prev = delnode->prev;
    rmiter(it);
    free(delnode);
    storage->size--;
    
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
    free(ind);
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
/// @brief пересортировка списка после изменения элемента
/// @param storage целевое хранилище
/// @param id идентификатор изменённого узла
void resort(Storage* storage, const ID_TYPE id){
    if(!storage || !storage->head) return;
    //завязка
    Node* node = storage->head;
    while(node != NULL && node->entry.id != id) {
        node = node->next;
    }
    if(!node) return;

    //
    if(node->prev) node->prev->next = node->next;
    else storage->head = node->next;

    if(node->next) node->next->prev = node->prev;
    else storage->tail = node->prev;

    node->next = NULL;
    node->prev = NULL;

    if(storage->head == NULL){
        storage->head = node;
        storage->tail = node;
        return;
    }

    Node* curr = storage->head;
    while(curr != NULL) {
        if(storage->comp(node->entry.var, curr->entry.var) < 0) break;
        curr = curr->next;
    }

    if(curr != NULL) {
        node->next = curr;
        node->prev = curr->prev;
        if(curr->prev) curr->prev->next = node;
        else storage->head = node;
        curr->prev = node;
    } else {
        node->prev = storage->tail;
        storage->tail->next = node;
        storage->tail = node;
    }
}

int8_t stchg(Storage* storage, const ID_TYPE id, changer ch, void* var){
    if(!storage || !ch)
        return ST_ARG_ERR;
    StEntry* entry = stgt(storage, id);
    if(!entry)
        return ST_NO_SUCH_ID;
    ch(entry->var, var);
    resort(storage, id);
    return ST_NOERR;
}

