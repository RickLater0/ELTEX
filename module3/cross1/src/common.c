#include "common.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>



ssize_t recv_full(int fd, void *buf, size_t n) {
    size_t left = n;
    ssize_t r;
    char *ptr = (char*)buf;
    while (left > 0) {
        r = recv(fd, ptr, left, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) return 0;
        left -= (size_t)r;
        ptr += r;
    }
    return (ssize_t)n;
}

ssize_t send_full(int fd, const void *buf, size_t n) {
    size_t left = n;
    ssize_t w;
    const char *ptr = (const char*)buf;
    while (left > 0) {
        w = send(fd, ptr, left, 0);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (w == 0) return 0;
        left -= (size_t)w;
        ptr += w;
    }
    return (ssize_t)n;
}

msg_t msg_creat(const char* uname, const char* content){
    msg_t msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.uname, uname, NAME_LEN);
    msg.uname[NAME_LEN - 1] = 0;
    strncpy(msg.content, content, CONTENT_LEN);
    msg.content[CONTENT_LEN - 1] = 0;
    return msg;
}

void msg_get(msg_t* msg, const char* buf){
    memset(msg, 0  , sizeof(msg_t)); 
    memcpy(msg, buf, sizeof(msg_t));
}

