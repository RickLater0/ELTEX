#ifndef SHARE_H
#define SHARE_H

#include <unistd.h>

#define CONTENT_LEN 512
#define FBUF_LEN 1024
#define SERV_PORT 45400
#define NAME_LEN 64
#define SYS_MSG_HEAD "SYS"

#define JOIN_MSG "Joined:"

#define CMD_EXIT "exit"
#define CMD_FILE "file" 
#define CMD_GET_USERS "users"

typedef struct {
    char uname  [NAME_LEN];
    char content[CONTENT_LEN];
} msg_t;

msg_t msg_creat(const char* uname, const char* content);
void msg_get(msg_t* msg, const char* buf);

ssize_t recv_full(int fd,       void *buf, size_t n);
ssize_t send_full(int fd, const void *buf, size_t n);

#endif