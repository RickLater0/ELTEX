#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <signal.h>
#include <poll.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pwd.h>

#include "share.h"

#define LISTEN_QUEUE_SIZE 5 //>= 2

typedef struct {
    int  fd;
    char uname[NAME_LEN];
} client_s;

int sockfd = -1;
struct sockaddr_in serv_addr;

char msgbuf[sizeof(msg_t)];
char fbuf  [FBUF_LEN];

struct pollfd fds[LISTEN_QUEUE_SIZE];
nfds_t nfds = 0;
client_s clients[LISTEN_QUEUE_SIZE - 1];


static void _ext(int sig){
    if(sig < 0)
        perror(NULL);
    else{
        printf("Shut down\n");
    }
    
    for (nfds_t i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }

    if(sockfd > 0)
        close(sockfd);
    
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void _client_accept(void){
    if(nfds >= LISTEN_QUEUE_SIZE){
        printf("Queue is loaded. No more clients can be accepted\n");
        return;
    }
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if(newsockfd < 0){
        perror("accept");
        return;
    }
    fds[nfds].fd = newsockfd;
    fds[nfds].events = POLLIN;
    nfds++;
    printf("New client accepted\n");
}

static void _remove_client(nfds_t n){
    printf("[SYSTEM] Client (fd=%d) disconnected.\n", fds[n].fd);
    close(fds[n].fd);
    memset(&clients[n - 1], 0, sizeof(client_s));
    for (nfds_t j = n; j < nfds - 1; j++) {
        fds[j] = fds[j + 1];
        clients[j - 1] = clients[j];
    }
    nfds--;
}

static void _relay_data(nfds_t n, const void* buffer, size_t sz){
    for(nfds_t i = 1; i < nfds; i++){
        if(i == n) continue;
        send_full(fds[i].fd, buffer, sz);
    }
}

//snprintf(notif, CONTENT_LEN, "%s:%s:%s:%ld", CMD_FILE, localfile, distfile, fsize); 
static void _relay_file(nfds_t n, const char* cmd){
    char local_fname[NAME_LEN];
    char dist_fname [NAME_LEN];
    off_t fsize = 0;

    if (sscanf(cmd, CMD_FILE ":%63[^:]:%63[^:]:%ld", local_fname, dist_fname, &fsize) != 3) {
        fprintf(stderr, "[SYSTEM] Invalid file notification format\n");
        return;
    }

    // 1. Защита Path Traversal: извлекаем только чистое имя файла
    char tmp_fname[NAME_LEN];
    strncpy(tmp_fname, dist_fname, sizeof(tmp_fname) - 1);
    tmp_fname[sizeof(tmp_fname) - 1] = '\0';
    char *safe_fname = basename(tmp_fname);

    // 2. Формируем путь сохранения в ~/Downloads/
    char target_path[1024];
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }

    if (home) {
        char downloads_dir[512];
        snprintf(downloads_dir, sizeof(downloads_dir), "%s/Downloads", home);
        mkdir(downloads_dir, 0755); // Создаем папку, если ее нет
        snprintf(target_path, sizeof(target_path), "%s/%s", downloads_dir, safe_fname);
    } else {
        snprintf(target_path, sizeof(target_path), "%s", safe_fname);
    }

    int fd = open(target_path, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fd < 0) {
        perror("open target file");
    }

    off_t remaining = fsize;
    char buffer[FBUF_LEN];

    msg_t meta_msg;
    strcpy(meta_msg.uname, SYS_MSG_HEAD);
    snprintf(meta_msg.content, CONTENT_LEN, "%s:%s:%s:%ld", CMD_FILE, local_fname, dist_fname, fsize);
    _relay_data(n, &meta_msg, sizeof(meta_msg));

    while (remaining > 0) {
        size_t to_read = (remaining < (off_t)sizeof(buffer)) ? (size_t)remaining : sizeof(buffer);
        ssize_t rb = recv_full(fds[n].fd, buffer, to_read);
        if (rb <= 0) {
            fprintf(stderr, "[SYSTEM] Connection lost while relaying file\n");
            break;
        }

        _relay_data(n, buffer, (size_t)rb);
        remaining -= rb;
    }

    if (remaining == 0) {
        printf("[SYSTEM] File relay complete: %s\n", dist_fname);
    }
}

static void _handle_cmd(nfds_t n, const char* cmd){

    if(strncmp(cmd, CMD_FILE, strlen(CMD_FILE)) == 0){
        _relay_file(n, cmd);
    }else if(strcmp(cmd, CMD_GET_USERS) == 0){
        msg_t msg;
        strcpy(msg.uname  , SYS_MSG_HEAD);
        int offset = snprintf(msg.content, CONTENT_LEN, "Users online: %lu\n", (unsigned long)(nfds - 1));
        
        for(nfds_t i = 0; i < nfds - 1; i++){
            if (offset < CONTENT_LEN && i != n) {
                offset += snprintf(msg.content + offset, (size_t)(CONTENT_LEN - offset > 0 ? CONTENT_LEN - offset : 0), 
                "  %lu. %s\n", i + 1, clients[i].uname);
            }
        }
        send_full(fds[n].fd, &msg, sizeof(msg));
    }else if(strncmp(cmd, JOIN_MSG, strlen(JOIN_MSG)) == 0){
        nfds_t cl_ind = n - 1;
        memset(&clients[cl_ind], 0, sizeof(client_s));
        strncpy(clients[cl_ind].uname, cmd + strlen(JOIN_MSG), NAME_LEN - 1);
        clients[cl_ind].uname[NAME_LEN - 1] = '\0';

        clients[cl_ind].fd = fds[n].fd;
    }
}

static int _handle_msg(nfds_t n){
    msg_t msg;
    int client_fd = fds[n].fd;
    ssize_t sz = recv_full(client_fd, &msg, sizeof(msg));
    
    if(sz <= 0){
        _remove_client(n);
        return 1;//удалить клиента
    }

    if(strcmp(SYS_MSG_HEAD, msg.uname) == 0)
        _handle_cmd(n, msg.content);
    else
        _relay_data(n, &msg, sizeof(msg));
    return 0;
}

int main(int argc, char* argv[]){
    
    if(argc != 1){
        printf("Usage: %s #(no arguments allowed)\n", argv[0]);
        return 0;
    }

    signal(SIGINT , _ext);
    signal(SIGTERM, _ext);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) _ext(-1);

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    printf("Starting...\n");
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERV_PORT);
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        _ext(-1);
    printf("Binded\n");
    if(listen(sockfd, LISTEN_QUEUE_SIZE) < 0)
        _ext(-1);
    printf("Listen\n");
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    nfds++;

    printf("Started\n");

    for(;;){
        if(poll(fds, nfds, -1) < 0){
            perror("poll");
            _ext(-1);
        }
        for(nfds_t i = 0; i < nfds; i++){
            if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)))
                continue;
            if(fds[i].fd == sockfd)
                _client_accept();
            else
                if(_handle_msg(i))
                    i--;
        }
    }
}
