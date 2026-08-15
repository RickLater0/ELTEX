#include "tcommon.h"
#include <stdio.h>
#include <sys/types.h>

void get_drrx_name(char *out, size_t size, pid_t ppid, pid_t pid) {
    (void)snprintf(out, size, "/tmp/taxi%d_dr-rx_%d.fifo", (int)ppid, (int)pid);
}

void get_drtx_name(char *out, size_t size, pid_t ppid, pid_t pid) {
    (void)snprintf(out, size, "/tmp/taxi%d_dr-tx_%d.fifo", (int)ppid, (int)pid);
}