#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
Реализовать очередь с приоритетом. Приоритет задается целым
числом от 0 до 255.
Реализовать функции для работы с очередью: 
1) добавление элемента и 
2) извлечение элемента: 
    находящегося первым в очереди/
    с указанным приоритетом/
    с приоритетом не ниже чем заданный. 
    
В тестирующей программе имитировать генерацию сообщений
с различным приоритетом и выборку данных с различными условиями
*/

#define INIT_CAPACITY 8
#define DEF_CP_KOEF 2.
#define NO_RESIZE_NEED 11

typedef struct Node
{
    PRIO_TYPE priority;
    void* data;
} node;

struct pqueue
{
    node* arr; 
    SIZE_TYPE cp;
    SIZE_TYPE sz;
};

static ERR_TYPE resize(pqueue* pq){
    if(!pq)
        return ARGERR;
    
    if(pq->cp == pq->sz){
        SIZE_TYPE newcp = pq->cp * DEF_CP_KOEF;
        node* newarr = realloc(pq->arr, newcp * sizeof(node));
        if(!newarr)
            return MEM_ALLOC_ERR;
        pq->arr = newarr;
        pq->cp = newcp;
        return NOERR;
    }else if
    (
        pq->cp / (2 * DEF_CP_KOEF) >= pq->sz && 
        pq->cp / (2 * DEF_CP_KOEF) >= INIT_CAPACITY
    ){
        SIZE_TYPE newcp = pq->cp * (1 / DEF_CP_KOEF);
        node* newarr = realloc(pq->arr, newcp * sizeof(node));
        if(!newarr)
            return MEM_ALLOC_ERR;
        pq->arr = newarr;
        pq->cp = newcp;
        return NOERR;
    }
    return NO_RESIZE_NEED;
}

ERR_TYPE mkq (pqueue** pq){
    if(!pq)
        return ARGERR;

    *pq = malloc(sizeof(pqueue));
    if(!*pq)
        return MEM_ALLOC_ERR;
    (*pq)->cp = INIT_CAPACITY;
    (*pq)->sz = 0;
    (*pq)->arr = malloc(sizeof(node) * INIT_CAPACITY);
    if(!(*pq)->arr)
        return MEM_ALLOC_ERR;
    return NOERR;
}

ERR_TYPE frq (pqueue* pq){
    if(pq){
        if(pq->arr){
            for(SIZE_TYPE i = 0; i < pq->sz; i++){
                free(pq->arr[i].data);
            }
            free(pq->arr);
        }
        free(pq);
        return NOERR;
    }
    return ARGERR;
}

SIZE_TYPE pqsz (pqueue* pq){
    if(pq)
        return pq->sz;
    return 0;
}

static ERR_TYPE rmind(pqueue* pq, SIZE_TYPE ind, void** data){
    *data = pq->arr[ind].data;

    if(ind != pq->sz-1)
        memmove(&pq->arr[ind], &pq->arr[ind + 1], sizeof(node) * (pq->sz - 1 - ind));

    pq->sz--;
    ERR_TYPE err = resize(pq);
    return NOERR;
}

ERR_TYPE enq  (pqueue* pq, void* data, PRIO_TYPE prio){
    if(!pq || !data)
        return ARGERR;

    node n;
    n.priority = prio;
    n.data = data;

    ERR_TYPE err = resize(pq);
    if(err < 0){
        return err;
    }

    SIZE_TYPE i = 0;
    while(i < pq->sz && pq->arr[i].priority > prio) {
        i++;
    }
    if(i < pq->sz) {
        memmove(&pq->arr[i + 1], &pq->arr[i], sizeof(node) * (pq->sz - i));
    }
    
    pq->arr[i] = n;
    pq->sz++;

    return NOERR;
}
ERR_TYPE deq  (pqueue* pq, void** data){
    if(!pq || !data)
        return ARGERR;
    if(pq->sz == 0)
        return NOT_FOUND;
    return rmind(pq, pq->sz-1, data);
}
//
ERR_TYPE pdeq (pqueue* pq, void** data, PRIO_TYPE prio){
    if(!pq || !data)
        return ARGERR;

    if(pq->sz == 0)
        return NOT_FOUND;

    for(SIZE_TYPE i = pq->sz; i > 0 ; i--){
        if(pq->arr[i - 1].priority == prio)
            return rmind(pq, i - 1, data);
    }

    *data = NULL;
    return NOT_FOUND;
}
ERR_TYPE spdeq(pqueue* pq, void** data, PRIO_TYPE prio){
    if(!pq || !data)
        return ARGERR;

    if(pq->sz == 0)
        return NOT_FOUND;

    for(SIZE_TYPE i = pq->sz; i > 0 ; i--){
        if(pq->arr[i - 1].priority >= prio)
            return rmind(pq, i - 1, data);
    }

    *data = NULL;
    return NOT_FOUND;
}

ERR_TYPE pqtostr(pqueue* pq, char** str) {
    if (!pq || !str)
        return ARGERR;

    if (pq->sz == 0) {
        *str = malloc(strlen("Очередь пуста") + 1);
        if (!*str)
            return MEM_ALLOC_ERR;
        strcpy(*str, "Очередь пуста");
        return NOERR;
    }

    size_t total_len = 0;

    for (SIZE_TYPE i = 0; i < pq->sz; i++) {
        const char* data_str = pq->arr[i].data ? (const char*)pq->arr[i].data : "NULL";
        
        // Получаем длину подстроки вида: [P:50, D: 'Важное уведомление']
        int len = snprintf(NULL, 0, "[P:%u, D: '%s']", pq->arr[i].priority, data_str);
        total_len += len;

        if (i < pq->sz - 1) {
            total_len += 4; 
        }
    }
    total_len += 1; 


    *str = malloc(total_len);
    if (!*str)
        return MEM_ALLOC_ERR;

    char* current_ptr = *str;
    size_t remaining_len = total_len;

    for (SIZE_TYPE i = 0; i < pq->sz; i++) {
        const char* data_str = pq->arr[i].data ? (const char*)pq->arr[i].data : "NULL";
        
        int len = snprintf(current_ptr, remaining_len, "[P:%u, D: '%s']", pq->arr[i].priority, data_str);
        current_ptr += len;
        remaining_len -= len;

        if (i < pq->sz - 1) {
            len = snprintf(current_ptr, remaining_len, "\n");
            current_ptr += len;
            remaining_len -= len;
        }
    }

    return NOERR;
}