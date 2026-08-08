#ifndef MEMBL_H
#define MEMBL_H

#include "err.h"
#include <stddef.h> 
#include <sys/types.h> 
#define data_t unsigned long long
#define DEF_MEM_SZ 4096
#define DEF_SEM_POSX_F "/sem_semproj.s"
#define DEF_SEM_SYSV_F "/tmp/sem_semproj.s"

typedef struct 
{
    size_t count;
    off_t  next_offset;
    data_t  data[];
} membl_s;

#endif //MEMBL_H