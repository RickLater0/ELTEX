#ifndef SHARIN_H
#define SHARIN_H

#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>
#include <sys/shm.h>

#include "lib/err.h"

#define NAME_LEN 256

typedef enum {
    SYSV,
    POS
} back_t;

typedef struct {
    back_t bk;
    size_t sz;
    void *addr;

    char name[NAME_LEN];
    int fd;

    key_t key;
    int shmid;
} sharin;

err shmem_create (sharin *shm, back_t bk, const char *name, key_t key, size_t sz, int mode);
err shmem_open   (sharin *shm, back_t bk, const char *name, key_t key, size_t sz);

err shmem_map    (sharin *shm, void **addr);
err shmem_unmap  (sharin *shm);

err shmem_destroy(sharin *shm);

#endif