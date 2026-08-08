#include "consumer.h"
#include "membl.h"
#include "sharin.h"

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



err consumer_main(int proj_id, const char* name){
    sharin sh;
    err r;
    
    if((r = shmem_open(&sh, SYSV, name, proj_id, DEF_MEM_SZ)) != NOERR)
        return r;
    if((r = shmem_map(&sh, &sh.addr)) != NOERR)
        return r;

    int sem;
    key_t key = ftok(DEF_SEM_F, proj_id);
    if((sem = semget(key, 1, 0666)) == -1){
        return SEM_GT_ERR;
    }

    srand((unsigned int)time(NULL));
    
    membl_s* blk = (membl_s*) sh.addr;
    struct sembuf sb_p = {0, -1, 0};
    struct sembuf sb_v = {0, 1, 0};

    while (1) 
    {
        if (semop(sem, &sb_p, 1) == -1) {
            shmem_unmap(&sh);
            return SEM_CR_ERR;
        }

        char proceesed = blk->count > 0 ? 1 : 0;
        if(blk->count > 0){
            data_t min_val = blk->data[0];
            data_t max_val = blk->data[0];
            for(size_t i = 0; i < blk->count; i++){
                if (blk->data[i] < min_val) min_val = blk->data[i];
                if (blk->data[i] > max_val) max_val = blk->data[i];
            }

            printf("Набор (смещение %zu): min = %llu, max = %llu\n", 
                (char*)blk - (char*)sh.addr,
                (unsigned long long)min_val, 
                (unsigned long long)max_val
            );
            blk->count = 0;
        }

        if (blk->next_offset == 0) {
            semop(sem, &sb_v, 1);
            break;
        }

        blk = (membl_s*) ((char*)sh.addr + blk->next_offset);

        semop(sem, &sb_v, 1);
        if(proceesed == 1){
            sleep((unsigned int)rand() % 4 + 1);
        }
    }
    
    shmem_unmap(&sh);
    return NOERR;
}