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
#include <semaphore.h>

static sharin sh;
static char* fname;
static sem_t* sem = NULL;

static void finally(){
    if(sem != SEM_FAILED){
        sem_close(sem);
        sem_unlink(DEF_SEM_POSX_F);
    }
    shmem_destroy(&sh);
}

static void siginth(int sig){
    (void)sig;
    finally();
    exit(EXIT_FAILURE);
}
err generator_main(int proj_id, const char* name){
    
    err r;
    fname = malloc(strlen(name) + 1);
    strcpy(fname, name);
    signal(SIGINT, siginth);
    if((r = shmem_create(&sh, POS, name, proj_id, DEF_MEM_SZ, O_CREAT | O_EXCL | O_RDWR | 0666)) != NOERR){
        finally();
        return r;
    }
        
    if((r = shmem_map(&sh, &sh.addr)) != NOERR){
        finally();
        return r;
    }

    sem = sem_open(DEF_SEM_POSX_F, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED) {
        finally();
        return SEM_CR_ERR;
    }
    srand((unsigned int)time(NULL));

    void *base = sh.addr;
    off_t free_offset = 0;

    sem_wait(sem);
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
    sem_post(sem);
    fprintf(stdout, "Gen completed\n");
    int all_done = 0;
    while (all_done != 1)
    {
        sem_wait(sem);

        
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

        sem_post(sem);

        sleep(5); 
    }
    //удаление семафора и разделяемой памяти
    finally();
    return NOERR;
}
