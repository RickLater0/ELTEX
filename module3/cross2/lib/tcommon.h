#ifndef TCOMMON_H
#define TCOMMON_H

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>

typedef struct drive{
    pid_t pid;
    int rx;
    int tx;
} driver_s;

typedef enum msg_code{
    BUSY,
    AVALIABLE,
    TO_WORK,
    GET_STATUS,
    WORK_IS_SET,
    MSG_ERR,
    TURNING_OFF
} msg_code_e;

typedef struct msg{
    msg_code_e code ;
    uint32_t   time ;
    pid_t      mypid;
} msg_t;

void get_drrx_name(char *out, size_t size, pid_t ppid, pid_t pid);
void get_drtx_name(char *out, size_t size, pid_t ppid, pid_t pid);

#endif