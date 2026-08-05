
#define RET_ERR -1
#define NOERR 0

#ifndef DARR_H
#define DARR_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct dynamic_arr
{
    
} darr;

int darr_init(darr* arr, size_t stsz);

int d_add(darr* arr, void* val);

int d_rm(darr* arr, int ind);

int d_gt(darr* arr, int ind, void* ret);

int d_er(darr* arr);

size_t d_sz(darr* arr);
#endif //DARR_H