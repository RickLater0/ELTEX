#include "Storage.h"
#include <string.h>
#include <stdlib.h>

#define INIT_CAPACITY 8
#define CAPACITY_K 2
#define ID_MAX UINT32_MAX
#define ID_MIN 1ull
//кринжовое хранилище (стоило назвать Collection или DB)
//и надо было писать эту задачу на PostgreSQL
typedef struct Storage{
    ID_TYPE size;
    ID_TYPE capacity;
    ID_TYPE curId;
    size_t stsz;
    StEntry* arr;
} Storage;
//итератор
typedef struct StIterator{
    Storage* storage;
    int8_t errCode; //1 - corrupted, 2 - end, 3 - 
    StEntry* node;
} StIterator;

/// @brief  создание хранилища
/// @param structsz размер хранимой структуры
/// @return возвращает указатель на созданное хранилище
Storage* mkstor (const size_t structsz){
    Storage* storage = (Storage*) malloc(sizeof(Storage));
    if(!storage)
        return NULL;

    storage->stsz     = structsz;
    storage->capacity = INIT_CAPACITY;
    storage->size     = 0;
    storage->curId    = 1;

    storage->arr = (StEntry*)calloc(storage->capacity, sizeof(StEntry));
    if(!storage->arr){
        free(storage);
        return NULL;
    }
    return storage;
}
/// @brief удаление хранилища
/// @param st указатель на удалаяемое хранилище
void     remstor(Storage* st){
    if(st){
        if(st->arr){
            for(ID_TYPE i = 0; i < st->size; i++) {
                if(st->arr[i].var) //освобождение памяти каждой ячейки, если она не NULL
                    free(st->arr[i].var);
            }
            free(st->arr);
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
/// @brief преобразование идентификатора Entry в индекс массива, где это Entry находится
/// @param storage 
/// @param id идентификатор
/// @return индекс в arr, если не найден, то ID_MAX
ID_TYPE idtoind(const Storage* const storage, const ID_TYPE id){
    if(id == ID_MAX || id < ID_MIN)
        return ID_MAX;
    for(ID_TYPE i = 0; i < storage->size; i++){
        if(storage->arr[i].id == id)
            return i;
    }
    return ID_MAX;
}
/// @brief Пересчёт текущего id для добавления. новый curId не может быть меньше текущего\
/// функция ищет ближайший свободный id справа
/// @param storage хранилище, у которого будет пересчёт id
void recid(Storage* storage){
    ID_TYPE r = ID_MAX; 
    ID_TYPE ncid = storage->curId + 1;
    ID_TYPE id = 0;
    ID_TYPE cid = storage->curId;
    for(ID_TYPE i = 0; i < storage->size; i++){
        id = storage->arr[i].id;
        if(id < cid - 1)
            continue;
        if(id - cid < r){
            r = id - cid;
            ncid = id + 1;
        }
    }
    storage->curId = ncid;
}
/// @brief поптытка изменить размер массива
/// @param storage 
/// @param k коэф. расширения (принят 2 и 1/2)
/// @return код ошибки 
int8_t resize(Storage* storage, float k){
    if(!storage)
        return -10 + ST_ARG_ERR;
    if(k < 0)
        return -10 + ST_ARG_ERR;
    ID_TYPE newcapacity = (ID_TYPE)(storage->capacity * k);
    if(newcapacity < INIT_CAPACITY) newcapacity = INIT_CAPACITY;
    //новая ёмкость не может быть меньше начальной

    if(newcapacity == storage->capacity || newcapacity < storage->size) 
        return 0;
    StEntry* newarr = (StEntry*)realloc(storage->arr, newcapacity * sizeof(StEntry));
    if(!newarr)
        return -10 + ST_MEM_ALLOC_ERR;
    storage->arr = newarr;
    storage->capacity = newcapacity;
    
    return 0;
}

/// @brief Вставка по id. Insert into st values (id, var).  
/// @param storage Вставка в хранилище по указателю
/// @param id Если id занят, вставка не произойдёт - ошибка ST_NO_SUCH_IND
/// @param var Указатель на вставляемое значение 
/// @return код ошибки
int8_t stin  (Storage* storage, const ID_TYPE id, void* var){
    if(!storage || !var)
        return ST_ARG_ERR;
    if(idtoind(storage, id) != ID_MAX)
        return ST_NO_SUCH_IND;//id занят
    if(storage->size >= storage->capacity){
        int8_t ret = resize(storage, CAPACITY_K);
        if(ret != 0)
            return ret;
    }

    storage->arr[storage->size].id = id;
    storage->arr[storage->size].var = malloc(storage->stsz);

    if(!storage->arr[storage->size].var) 
        return ST_MEM_ALLOC_ERR;
    memcpy(storage->arr[storage->size].var, var, storage->stsz);
    storage->size++;
    recid(storage);

    return 0;
}
/// @brief Вставка с автоматическим указанием id. Insert into st values (var).
/// @param storage Вставка в хранилище по указателю
/// @param var Вставляемое значение
/// @return код ошибки
int8_t stil (Storage* storage, void* var){
    return stin(storage, storage->curId, var);
};


/// @brief Удаление по id. Delete from st where st.id = ? 
/// @param storage Удаление из хранилища
/// @param id Идентификатор удаляемой записи
/// @return код ошибки
int8_t strm     (Storage* storage, const ID_TYPE id){
    if(!storage)
        return ST_ARG_ERR;
    ID_TYPE ind = idtoind(storage, id);
    if(ind == ID_MAX)
        return ST_NO_SUCH_IND;

    if(storage->arr[ind].var)//удаление элемента
        free(storage->arr[ind].var);
    
    if (ind < storage->size - 1) {//сдвиг по фазе
        memmove(&storage->arr[ind], &storage->arr[ind + 1], (storage->size - ind - 1) * sizeof(StEntry));
    }

    storage->size--;//уменьшение размера
    //по приколу удаляем предыдущую запись
    memset(&storage->arr[storage->size], 0, sizeof(StEntry));
    //попытка уменьшения размера
    if(storage->size > INIT_CAPACITY && storage->size <= storage->capacity / 4){
        int8_t ret = resize(storage, 1. / CAPACITY_K);
        if(ret != 0)
            return ret;
    }
    return 0;
    
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
        return ST_NO_SUCH_IND;
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
    ID_TYPE ind = idtoind(storage, id);
    if(ind == ID_MAX)
        return NULL;

    return &storage->arr[ind];
}
/// @brief SELECT * FROM st WHERE var = ? LIMIT 1
/// @param storage целевое хранилище
/// @param comp указатель на компаратор
/// @param var целевое значение
/// @return Первая подходящая запись. Не найдена - NULL
ID_TYPE  stff (const Storage* const storage, comparator comp, const void* const var){
    if(!storage || !comp || !var)
        return ID_MAX;

    for(ID_TYPE i = 0; i < storage->size; i++){
        void* elem = storage->arr[i].var;
        if(comp(elem, var) == 0){
            return storage->arr[i].id;
        } 
    }
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

    for(ID_TYPE i = 0; i < storage->size; i++){
        void* elem = storage->arr[i].var;
        if(comp(elem, var) == 0){
            ids[*retsz] = storage->arr[i].id;
            *retsz = *retsz + 1;
        }
    }
    if(*retsz == 0){
        free(ids);
        return NULL;
    }

    ID_TYPE* newids = (ID_TYPE*)realloc(ids, *retsz * sizeof(ID_TYPE));
    if(!newids){
        return NULL;
    }
        

    ids = newids;
    return ids;
}

/// @brief Установка итератора в начало. Возвращает итератор
/// @param st целевое хранилище
/// @return итератор
StIterator* stbeg (Storage* st){
    if(!st)
        return NULL;

    StIterator* it = malloc(sizeof(StIterator));
    if(!it)
        return NULL;
    it->storage = st;
    if(st->size == 0){
        it->errCode = 1;
        it->node = NULL;
    }else{
        it->errCode = 0;
        it->node = st->arr;
    }
}
/// @brief Установка итератора в конец. Возвращает итератор
/// @param st целевое хранилище
/// @return итератор
StIterator* stend (Storage* st){
    if(!st)
        return NULL;

    StIterator* it = malloc(sizeof(StIterator));
    if(!it)
        return NULL;

    it->storage = st;
    if(st->size == 0){
        it->errCode = 1;
        it->node = NULL;
    }else{
        it->errCode = 2;
        it->node = NULL;
    }
}
/// @brief Устанавливает итератор на следующую позицию
/// @param it Указатель на итератор
void stnext(StIterator* it){
    if(it->errCode == 2 || it->errCode == 1){
        it->errCode = 1;
        it->node = NULL;
        return;
    }
    if(it->errCode == 3){
        it->errCode = 0;
        it->node = it->storage->arr;
        return;
    }
    it->node++;
    if(it->node >= it->storage->arr + it->storage->size){
        it->errCode = 2;
        it->node = NULL;
    }
}
/// @brief Устанавливает итератор на предыдущую позицию
/// @param it Указатель на итератор
void stprev(StIterator* it){
    if(it->errCode == 1 || it->errCode == 3){
        it->errCode = 1;
        it->node = NULL;
        return;
    }
    
    if(it->errCode == 2){
        it->errCode = 0;
        it->node = it->storage->arr + it->storage->size - 1;
        return;
    }
    it->node--;
    if(it->node < it->storage->arr){
        it->errCode = 3;
        it->node = NULL;
    }
}
/// @brief Разыменовывание итератора
/// @param it Указатель на итератор
/// @return запись в коллекции вместе с id
StEntry* stitgt(const StIterator* const it){
    if (!it || it->errCode != 0) return NULL;
    return it->node;
}
/// @brief удаление итератора
/// @param it указатель на итератор
void        rmiter(StIterator* it){
    if(it)
        free(it);
}

int8_t itcomp(const StIterator* const it1, const StIterator* const it2){
    if(it1 == it2)
        return 0;
    if(!it1 || !it2)
        return ST_ARG_ERR;
    return it1->node < it2->node ? -1 : (it1->node == it2->node ? 0 : 1); 
}