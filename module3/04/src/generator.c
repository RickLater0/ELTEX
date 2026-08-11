#include "sharin.h"
#include "generator.h"
#include "membl.h"


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
#include <errno.h>
#include <stdio.h>
#include <signal.h>

sharin sh;

static void finally(int sem, const char* name){
    shmem_unmap(&sh);
    shmem_destroy(&sh);
    semctl(sem, 0, IPC_RMID);
    remove(DEF_SEM_F);
    remove(name);
}

char* fname;
int sem = -1;

static void siginth(int sig){
    (void)sig;
        if (sem != -1) {
        finally(sem, fname);
    } else {
        shmem_unmap(&sh);
        shmem_destroy(&sh);
        remove(DEF_SEM_F);
        remove(fname);
    }
    exit(EXIT_FAILURE);
}
err generator_main(int proj_id, const char* name){
    
    err r;
    fname = malloc(strlen(name) + 1);
    strcpy(fname, name);
    signal(SIGINT, siginth);
    if((r = shmem_create(&sh, SYSV, name, proj_id, DEF_MEM_SZ, IPC_EXCL | 0666)) != NOERR)
        return r;
    if((r = shmem_map(&sh, &sh.addr)) != NOERR)
        return r;

    
    int fd;
    if((fd = open(DEF_SEM_F, O_CREAT | O_RDWR | 0666)) == -1){
        if(errno == EEXIST)
            return FEXISTS;
        return OPEN_ERR;
    }
            
    close(fd);
    key_t key = ftok(DEF_SEM_F, proj_id);
    if((sem = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) == -1){
        finally(sem, name);
        return SEM_CR_ERR;
    }

    // Устанавливаем значение семафора = 1 
    if (semctl(sem, 0, SETVAL, 1) == -1) {
        finally(sem, name);
        return SEM_CR_ERR;
    }

    srand((unsigned int)time(NULL));

    void *base = sh.addr;
    off_t free_offset = 0;

    struct sembuf sb_p = {0, -1, 0};
    struct sembuf sb_v = {0,  1, 0};//сделать как в презентации
    semop(sem, &sb_p, 1);
    while (1)
    {
        size_t count = ((size_t)rand()) % 20 + 1;
        size_t blk_sz = sizeof(membl_s) + count * sizeof(data_t);
    
        if((size_t)free_offset + blk_sz > sh.sz)
            break;
        
        off_t next_offset = free_offset + (off_t)blk_sz;
        
        if((size_t)next_offset + (sizeof(membl_s) + sizeof(data_t)) > sh.sz){
            next_offset = 0;
        }

        membl_s* blk = (membl_s*)((char*)base + free_offset);
        blk->count = count;
        blk->next_offset = next_offset;
        for(size_t i = 0; i < count; i++){
            blk->data[i] = ((data_t)rand() | (data_t)rand() << (sizeof(data_t) * 4));
        }

        free_offset = next_offset;
        if (next_offset == 0)
            break;
    }
    semop(sem, &sb_v, 1);
    fprintf(stdout, "Gen completed\n");
    int all_done = 0;
    while (all_done != 1)
    {
        if (semop(sem, &sb_p, 1) == -1) {
            finally(sem, name);
            return SEM_CR_ERR;
        }

        
        off_t offset = 0;
        membl_s *b = (membl_s*)base;
        while (b->next_offset != 0) {
            if (b->count != 0) {
                all_done = 0;
                break;
            }
            offset = b->next_offset;
            b = (membl_s*)((char*)base + offset);
        }
        // после цикла, если не было break и мы прошли все блоки
        if (b->next_offset == 0 && b->count == 0) {
            all_done = 1;
        }

        semop(sem, &sb_v, 1);

        sleep(5); 
    }
    //удаление семафора и разделяемой памяти
    finally(sem, name);
    return NOERR;
}
