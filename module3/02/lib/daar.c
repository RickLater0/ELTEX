
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/darr.h"

#define BASE_CAPACITY 8

typedef struct dynamic_arr
{
    void* arr;
    size_t stsz;
    size_t sz;
    size_t capacity;
} darr;

int darr_init(darr* arr, size_t stsz){
    if(arr == NULL || stsz == 0)
        return RET_ERR;

    arr->arr = malloc(BASE_CAPACITY * stsz);
    if(arr->arr == NULL){
        return RET_ERR;
    }
    arr->stsz = stsz;
    arr->capacity = BASE_CAPACITY;
    arr->sz = 0;
    return NOERR;
}

int d_add(darr* arr, void* val){
    if(arr == NULL || val == NULL)
        return RET_ERR;

    if(arr->sz >= arr->capacity){
        size_t new_capacity = arr->capacity * 2;
        void* new_arr = realloc(arr->arr, new_capacity);

        if(new_arr == NULL)
            return RET_ERR;
        
        arr->capacity = new_capacity;
        arr->arr = new_arr; 
    }

    memcpy((char*)arr->arr + (arr->stsz * arr->sz), val, arr->stsz);
    arr->sz++; 
    return NOERR;
}

int d_rm(darr* arr, int ind){
    if(arr == NULL)
        return RET_ERR;

    if(ind < 0 || ind > arr->sz)
        return RET_ERR;
    
    memmove(
        (char*)arr->arr + (arr->stsz * ind),
        (char*)arr->arr + (arr->stsz * (ind+1)),
        (arr->sz - ind - 1) * arr->stsz
    );

    memset((char*)arr->arr + (arr->stsz * arr->sz), 0, arr->stsz);
    arr->sz--;
    if(arr->sz < arr->capacity / 4 && arr->capacity / 4 > BASE_CAPACITY){
        size_t new_capacity = arr->capacity / 2;
        void* new_arr = realloc(arr->arr, new_capacity);

        if(new_arr == NULL)
            return RET_ERR;
        
        arr->capacity = new_capacity;
        arr->arr = new_arr; 
    }
    return NOERR;
}

int d_gt(darr* arr, int ind, void* ret){
    if(arr == NULL)
        return RET_ERR;

    if(ind < 0 || ind > arr->sz)
        return RET_ERR;

    memcpy(ret, (char*)arr->arr + ind * arr->stsz, arr->stsz);

    return NOERR;
}

int d_er(darr* arr){
    if(arr == NULL)
        return RET_ERR;
    if(arr->arr != NULL)
        free(arr->arr);
    return NOERR;
}

size_t d_sz(darr* arr){
    if(arr == NULL)
        return 0;
    return arr->sz;
}