#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <signal.h>
#include <pwd.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>

#include "share.h"

int sockfd = -1;
struct sockaddr_in serv_addr;
struct hostent *serv;

char inbuf[sizeof(msg_t)], outbuf[sizeof(msg_t)];
char username[NAME_LEN] = "John Doe";

static ssize_t _msg_send(const char* uname, const char* content){
    msg_t msg = msg_creat(uname, content);
    memcpy(outbuf, &msg, sizeof(msg_t));
    return send(sockfd, outbuf, sizeof(msg_t), 0);
}

static void _ext(int sig){
    if(sig < 0)
        perror(NULL);

    if(sig >= 0){
        char _msg_goodbye_content[CONTENT_LEN];
        snprintf(_msg_goodbye_content, CONTENT_LEN, "User %s left chat", username);
        _msg_send(SYS_MSG_HEAD, _msg_goodbye_content);
    }
    
    if(sockfd > 0)
        close(sockfd);
    
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void _share_file(char local_fname[], char dist_fname[]){
    
    int fd = open(local_fname, O_RDONLY); 
    if(fd < 0){
        perror("open file");
        return;
    }

    //получение размера файла
    off_t fsize = lseek(fd, 0, SEEK_END); 
    lseek(fd, 0, SEEK_SET);               

    //отправка метаинформации
    char notif[CONTENT_LEN];
    snprintf(notif, CONTENT_LEN, "%s:%s:%s:%ld", CMD_FILE, local_fname, dist_fname, fsize); 
    if (_msg_send(SYS_MSG_HEAD, notif) < 0) {
        perror("send notification");
        close(fd);
        return;
    }

    off_t remaining = fsize;
    char buffer[FBUF_LEN]; 
    ssize_t bytes_read;

    //чтение данных из файла до его окончания
    while((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) { 
        if(send_full(sockfd, buffer, (size_t)bytes_read) != bytes_read)
            break;
        remaining -= (off_t)bytes_read;
    }
    close(fd);

    if (remaining == 0)
        printf("[SYSTEM] File %s sent (%ld bytes)\n", local_fname, fsize); 
    else
        printf("[SYSTEM] Failed to send complete file\n"); 
}

static void _recv_file(const char *notification){
    char local_fname[NAME_LEN]; 
    char dist_fname[NAME_LEN];  
    off_t fsize = 0;
    
    //парсинг метаинформации
    if (sscanf(notification, CMD_FILE ":%63[^:]:%63[^:]:%ld", local_fname, dist_fname, &fsize) != 3) {
        fprintf(stderr, "Invalid file notification format\n");
        return;
    }

    //создание файла
    int fd = open(dist_fname, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
    if (fd < 0) {
        fprintf(stderr, "CHILD CANNOT CREATE FILE: %s\n", dist_fname); 
    }

    off_t remaining = fsize;
    char buffer[FBUF_LEN]; 

    //вычитываем из сокета точное количество байт
    while (remaining > 0) { 
        //определение размера чтения
        size_t to_read = (remaining < (off_t)sizeof(buffer)) ? (size_t)remaining : sizeof(buffer); 
        
        ssize_t rb = recv_full(sockfd, buffer, to_read); 
        if (rb <= 0) {
            fprintf(stderr, "Connection lost while receiving file\n");
            break;
        }
        
        // Если файл открыт, записываем данные
        if (fd >= 0) {
            size_t written = 0;
            while (written < (size_t)rb) {
                ssize_t w = write(fd, buffer + written, (size_t)rb - written); 
                if (w <= 0) break;
                written += (size_t)w;
            }
        }
        remaining -= rb; 
    }

    if (fd >= 0) {
        close(fd);
        printf("[SYSTEM] File saved as %s (%ld bytes)\n", dist_fname, fsize);
    }
}

const char _space[] = " \t";

static void _exec_cmd(char input[]){
    size_t offs = input[0] == '/' ? 1 : 0;
    char *cmd = strtok(input + offs, _space);
    if (!cmd) return;

    if(strcmp(cmd, CMD_EXIT) == 0)
        _ext(0);
    else if(strcmp(cmd, CMD_FILE) == 0){
        char *localfile = strtok(NULL, _space);
        char *distfile = strtok(NULL, _space); 
        
        if (!localfile || !distfile){
            fprintf(stdout, "/%s <LOCAL_FILENAME> <DIST_FILENAME>\n", CMD_FILE);
            fflush(stdout); 
            return;
        }
            
        _share_file(localfile, distfile); 
    }else if(strcmp(cmd, CMD_GET_USERS) == 0){
        _msg_send(SYS_MSG_HEAD, CMD_GET_USERS);
    }
}

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Usage: %s <SERV_IP_ADDR>\n", argv[0]);
        return 0;
    }

    static char *server_ip;
    server_ip = argv[1];

    signal(SIGINT , _ext);
    signal(SIGTERM, _ext);

    struct passwd *pw = getpwuid(getuid());
    if (pw)
        strncpy(username, pw->pw_name, sizeof(username) - 1);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        _ext(-1);

    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        _ext(-1);
    }

    if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        _ext(-1);

    char _msg_hello_content[CONTENT_LEN];
    snprintf(_msg_hello_content, CONTENT_LEN, JOIN_MSG "%s", username);
    _msg_send(SYS_MSG_HEAD, _msg_hello_content);

    fprintf(stdout, "-==Hello-chat==-\n You are %s\n\n", username);
    fflush(stdout);

    fd_set read_fds;
    fd_set master;
    FD_ZERO(&read_fds);
    FD_ZERO(&master);

    int FD_MAX = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

    FD_SET(sockfd, &master);
    FD_SET(STDIN_FILENO, &master);

    while(1) {
        read_fds = master;
        if (select(FD_MAX + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            _ext(-1);
        }
        if (FD_ISSET(sockfd, &read_fds)) {
            msg_t msg;
            ssize_t n = recv_full(sockfd, &msg, sizeof(msg_t));
            if (n <= 0) {
                if (n == 0)
                    printf("Server closed connection.\n");
                else
                    perror("recv");
                _ext(0);
            }

            // Вывод полученного сообщения
            if (strcmp(msg.uname, SYS_MSG_HEAD) == 0) { 
                if (strncmp(msg.content, CMD_FILE, strlen(CMD_FILE)) == 0) {
                    printf("[SYSTEM] Incoming file transfer...\n");
                    _recv_file(msg.content);
                } else {
                    printf("[SYSTEM] %s\n", msg.content);
                }
            } else {
                printf("[%s]: %s\n", msg.uname, msg.content);
            }
            fflush(stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input[CONTENT_LEN];
            if (fgets(input, CONTENT_LEN, stdin) == NULL) {
                // EOF (Ctrl+D)
                _ext(0);
            }
            input[strcspn(input, "\n")] = '\0';

            if(input[0] == '/')
                _exec_cmd(input);

            // Отправка обычного сообщения
            else if (strlen(input) > 0) {
                if (_msg_send(username, input) < 0) 
                    _ext(-1);
            }
        }

    }
    return 0;
}