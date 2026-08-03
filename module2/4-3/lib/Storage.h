#ifndef STORAGE_H
#define STORAGE_H
#include <stdint.h>
#include <stddef.h>
#define ID_TYPE uint32_t


#define ST_NO_SUCH_ID -2
#define ST_MEM_ALLOC_ERR -3
#define ST_ARG_ERR -4
#define ST_BUSY_ID -5
#define ST_NOERR 0

typedef struct StEntry{
    ID_TYPE id;
    void* var;
} StEntry;

typedef struct Storage Storage;

typedef struct StIterator StIterator;

typedef int8_t  (*comparator) (const void* const, const void* const);
typedef int8_t  (*changer)    (void*, void*);
typedef void (*st_visit_func)(ID_TYPE id, void* var, int depth, int is_left);

void st_visit_tree(Storage* st, st_visit_func func);
Storage* mkstor (const size_t structsz);
void     remstor(Storage*);
ID_TYPE  stsize (const Storage* const);

int8_t stil (Storage*, void*);
//int8_t stin (Storage*, const ID_TYPE, void*);

int8_t strm (Storage*, const ID_TYPE);
int16_t ster (Storage*, comparator, void*);

StEntry* stgt (const Storage* const, const ID_TYPE);
ID_TYPE  stff (const Storage* const, comparator, const void* const);
ID_TYPE* stfa (const Storage* const, comparator, const void* const, ID_TYPE* retsz);
int8_t stchg(Storage* storage, const ID_TYPE id, changer ch, void* var);


StIterator* stbeg (const Storage* const st);
StIterator* stend (const Storage* const st);

void stnext(StIterator*);
void stprev(StIterator*); 
StEntry*    stitgt(const StIterator* const);
void        rmiter(StIterator*);
int8_t      itcomp(const StIterator* const, const StIterator* const);
#endif