#ifndef ERR_H
#define ERR_H

typedef enum {
    NOERR,
    ARGERR,
    MEM_ALLOC_ERR,
    OPEN_ERR,
    MAP_ERR,
    UNMAP_ERR,
    DESTROY_ERR,
    SEM_CR_ERR
} err;

#endif //ERR_H
