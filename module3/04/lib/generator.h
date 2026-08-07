#ifndef GENERATOR_H
#define GENERATOR_H

#include "lib/sharin.h"
#include "lib/generator.h"
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
#include "lib/membl.h"

static data_t gen_data(){
    return ((data_t)rand() | (data_t)rand() << (sizeof(data_t) * 8 / 2)); 
}

err gen_main(int proj_id, const char* name){
    sharin sh;
    err r;
    
    if((r = shmem_create(&sh, SYSV, name, proj_id, DEF_MEM_SZ, IPC_EXCL | 0666)) != NOERR)
        return r;
    if((r = shmem_map(&sh, NULL)) != NOERR)
        return r;

    int sem;
    key_t key = ftok(DEF_SEM_F, proj_id);
    if((sem = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        shmem_destroy(&sh);
        return SEM_CR_ERR;
    }

    // Устанавливаем значение семафора = 1 
    if (semctl(sem, 0, SETVAL, 1) == -1) {
        semctl(sem, 0, IPC_RMID);
        shmem_destroy(&sh);
        return SEM_CR_ERR;
    }

    srand(time(NULL));

    void *base = sh.addr;
    size_t free_offset = 0;

    while (1)
    {
        size_t count = ((size_t)rand()) % 20 + 1;
        size_t blk_sz = sizeof(membl_s) + count * sizeof(data_t);
    
        if(free_offset + blk_sz > sh.sz)
            break;
        
        size_t next_offset = free_offset + blk_sz;
        
        if(next_offset + (sizeof(membl_s) + sizeof(data_t)) > sh.sz){
            next_offset = 0;
        }

        membl_s* blk = (membl_s*)((char*)base + free_offset);
        blk->count = count;
        blk->next_offset = next_offset;
        for(size_t i = 0; i < count; i++){
            blk->data[i] = gen_data();
        }

        free_offset = next_offset;
        if (next_offset == 0)
            break;
    }
    int all_done = 0;
    while (all_done != 1)
    {
        struct sembuf sb = {0, -1, 0};   // P
        if (semop(sem, &sb, 1) == -1) {
            shmem_unmap(&sh);
            shmem_destroy(&sh);
            return SEM_CR_ERR;
        }

        
        size_t offset = 0;
        membl_s *b;
        while (b->next_offset != 0) {
            b = (membl_s*)((char*)base + offset);
            if (b->count != 0) {
                all_done = 0;
                break;
            }
            offset = b->next_offset;
        }

        struct sembuf sb_v = {0, 1, 0};   // V
        semop(sem, &sb_v, 1);

        sleep(5); 
    }
    //удаление семафора и разделяемой памяти
    semctl(sem, 0, IPC_RMID);
    shmem_unmap(&sh);
    shmem_destroy(&sh);
    return NOERR;
}

#endif