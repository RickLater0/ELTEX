#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

#define PRIO_TYPE uint8_t
#define ERR_TYPE  int8_t
#define SIZE_TYPE uint32_t

#define LOWEST_PRIO  255
#define HIGHEST_PRIO  0 

#define NOERR          0
#define ARGERR        -10
#define MEM_ALLOC_ERR -11
#define NOT_FOUND      10

typedef struct pqueue pqueue;

ERR_TYPE  mkq  (pqueue**);
ERR_TYPE  frq  (pqueue* );

SIZE_TYPE pqsz (pqueue* );

ERR_TYPE  enq  (pqueue*, void*,  PRIO_TYPE);
ERR_TYPE  deq  (pqueue*, void**);
ERR_TYPE  pdeq (pqueue*, void**, PRIO_TYPE);
ERR_TYPE  spdeq(pqueue*, void**, PRIO_TYPE);

ERR_TYPE  pqtostr(pqueue*, char**);

#endif //QUEUE_H