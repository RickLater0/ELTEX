#ifndef ERR_H
#define ERR_H

typedef enum {
    NOERR = 0,
    ARGERR,
    MEM_ALLOC_ERR,
    OPEN_ERR,
    MAP_ERR,
    UNMAP_ERR,
    DESTROY_ERR,
    SEM_CR_ERR,
    SEM_GT_ERR,
    FEXISTS,
    HTBL_OK = 0,
    HTBL_ERR_PARAM,
    HTBL_ERR_NOMEM,
    HTBL_ERR_NOT_FOUND
} err;

#endif