#ifndef CONSUMER_H
#define CONSUMER_H

#include "lib/sharin.h"
#include "lib/consumer.h"
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "lib/membl.h"

err consumer_main(int proj_id, const char* name){
    sharin sh;
    err r;
    
    if((r = shmem_open(&sh, SYSV, name, proj_id, DEF_MEM_SZ)) != NOERR)
        return r;
    if((r = shmem_map(&sh, NULL)) != NOERR)
        return r;

    int sem;
    key_t key = ftok(DEF_SEM_F, proj_id);
    if((sem = semget(key, 1, 0666)) == -1){
        shmem_destroy(&sh);
        return SEM_CR_ERR;
    }

    srand(time(NULL));
    
    membl_s* blk = (membl_s*) sh.addr;
    struct sembuf sb_p = {0, -1, 0};
    struct sembuf sb_v = {0, 1, 0};

    ///TODO:Переделать логику итерации в цикле
    for(;;)
    {
        if (semop(sem, &sb_p, 1) == -1) {
            shmem_unmap(&sh);
            return SEM_CR_ERR;
        }

        if(blk->next_offset == 0){
            semop(sem, &sb_v, 1);
        }

        if(blk->count == 0){
            blk = (membl_s*) ((char*)sh.addr + blk->next_offset);
            continue;
        }

        data_t min_val = blk->data[0];
        data_t max_val = blk->data[0];
        for(size_t i = 0; i < blk->count; i++){
            if (blk->data[i] < min_val) min_val = blk->data[i];
            if (blk->data[i] > max_val) max_val = blk->data[i];
        }

        printf("Набор (смещение %zu): min = %llu, max = %llu\n", 
            blk->next_offset - (sizeof(membl_s) + blk->count * sizeof(data_t)), (unsigned long long)min_val, (unsigned long long)max_val);
        blk->count = 0;

        

        semop(sem, &sb_v, 1);
        sleep(rand() % 2 + 1);
    }
    
    shmem_unmap(&sh);
    return NOERR;
}

#endif