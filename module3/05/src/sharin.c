#define _POSIX_C_SOURCE 200809L

#include "sharin.h"

#include <errno.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

err shmem_create(sharin *shm, back_t bk, const char *name, int proj_id, size_t sz, int mode) {
    if (!shm) return ARGERR;
    
    memset(shm, 0, sizeof(sharin));
    shm->bk = bk;
    shm->sz = sz;

    if (bk == POS) {
        if (!name) return ARGERR;
        strncpy(shm->name, name, NAME_LEN - 1);
        
        shm->fd = shm_open(shm->name, O_CREAT | O_RDWR, (unsigned int)mode);
        if (shm->fd == -1) {
            if (errno == EEXIST) return FEXISTS;
            return OPEN_ERR;
        }

        if (ftruncate(shm->fd, (off_t)sz) == -1) {
            close(shm->fd);
            return MEM_ALLOC_ERR;
        }
    } else if (bk == SYSV) {
        int fd;
        if((fd = open(name, O_CREAT | O_EXCL)) == -1){
            if(errno == EEXIST)
                return FEXISTS;
            return OPEN_ERR;
        }
            
        close(fd);

        key_t qkey = ftok(name, proj_id);
        if(qkey == -1){
            return OPEN_ERR;
        }

        shm->key = qkey;
        shm->shmid = shmget(shm->key, sz, IPC_CREAT | mode);
        if (shm->shmid == -1) return OPEN_ERR;
    } else {
        return ARGERR;
    }
    return NOERR;
}

err shmem_open(sharin *shm, back_t bk, const char *name, int proj_id, size_t sz) {
    if (!shm) return ARGERR;

    memset(shm, 0, sizeof(sharin));
    shm->bk = bk;
    shm->sz = sz;

    if (bk == POS) {
        if (!name) return ARGERR;
        strncpy(shm->name, name, NAME_LEN - 1);
        
        shm->fd = shm_open(shm->name, O_RDWR, 0666);
        if (shm->fd == -1) return OPEN_ERR;
    } else if (bk == SYSV) {
        key_t qkey = ftok(name, proj_id);
        if(qkey == -1){
            return OPEN_ERR;
        }

        shm->key = qkey;
        shm->shmid = shmget(shm->key, sz, 0666);
        if (shm->shmid == -1) return OPEN_ERR;
    } else {
        return ARGERR;
    }
    return NOERR;
}

err shmem_map(sharin *shm, void **addr) {
    if (!shm || !addr) return ARGERR;
    
    if (shm->addr != NULL) {
        *addr = shm->addr;
        return NOERR;
    }

    if (shm->bk == POS) {
        shm->addr = mmap(NULL, shm->sz, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fd, 0);
        if (shm->addr == MAP_FAILED) {
            shm->addr = NULL;
            *addr = NULL;
            return MAP_ERR;
        }
    } else if (shm->bk == SYSV) {
        shm->addr = shmat(shm->shmid, NULL, 0);
        if (shm->addr == (void*)-1) {
            shm->addr = NULL;
            *addr = NULL;
            return MAP_ERR;
        }
    } else {
        *addr = NULL;
        return ARGERR;
    }

    *addr = shm->addr;
    return NOERR;
}

err shmem_unmap(sharin *shm) {
    if (!shm || shm->addr == NULL) return ARGERR;

    int res = 0;
    if (shm->bk == POS) {
        res = munmap(shm->addr, shm->sz);
        close(shm->fd);
    } else if (shm->bk == SYSV) {
        res = shmdt(shm->addr);
    } else {
        return ARGERR;
    }

    if (res == -1) return UNMAP_ERR;

    shm->addr = NULL;
    return NOERR;
}

err shmem_destroy(sharin *shm) {
    if (!shm) return ARGERR;

    if (shm->addr != NULL) {
        shmem_unmap(shm);
    }

    int res = -1;
    if (shm->bk == POS) {
        res = shm_unlink(shm->name);
    } else if (shm->bk == SYSV) {
        res = shmctl(shm->shmid, IPC_RMID, NULL);
        remove(shm->name);
    } else {
        return ARGERR;
    }

    return (res == -1) ? DESTROY_ERR : NOERR;
}