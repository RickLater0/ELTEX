#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <pwd.h>
#include <signal.h>

#define CONTENT_LEN 512
#define IN_PORT 45400
#define BROADCAST_IP "192.168.56.255"
#define NAME_LEN 64
#define SYS_MSG_HEAD "SYS"

typedef struct {
    char uname[NAME_LEN];
    char content[CONTENT_LEN];
} msg_t;


char username[NAME_LEN] = "John Doe";
int sockfd = -1;
struct sockaddr_in recvaddr, broadaddr;
char inbuf[sizeof(msg_t)], outbuf[sizeof(msg_t)];
int broad_perm = 1;
int reuse_perm = 1;
pid_t child_pid = -1;

static void _ext(int sig);

static msg_t _msg_creat(const char* uname, const char* content){
    msg_t msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.uname, uname, NAME_LEN);
    msg.uname[NAME_LEN - 1] = 0;
    strncpy(msg.content, content, CONTENT_LEN);
    msg.content[CONTENT_LEN - 1] = 0;
    return msg;
}

static ssize_t _msg_send(const char* uname, const char* content){
    msg_t msg = _msg_creat(uname, content);
    memcpy(outbuf, &msg, sizeof(msg_t));
    return sendto(sockfd, outbuf, sizeof(msg_t), 0, (struct sockaddr*)&broadaddr, sizeof(broadaddr));
}

static void _msg_get(msg_t* msg, const char* buf){
    memcpy(msg, buf, sizeof(msg_t));
}

static void _ext(int sig){
    if(sig < 0)
        perror(NULL);

    if(sig >= 0 && child_pid > 0){
        kill(child_pid, SIGTERM);

        char tmp[CONTENT_LEN];
        snprintf(tmp, CONTENT_LEN, "User %s left chat", username);
        _msg_send(SYS_MSG_HEAD, tmp);
    }
    
    if(sockfd > 0)
        close(sockfd);
    
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    (void) argv;
    if(argc > 1){
        fprintf(stdout, "Arguments not allowed\n");
        return E2BIG;
    }
    //автозакрывалка
    signal(SIGINT, _ext);
    //получение системного имени пользователя
    struct passwd *pw = getpwuid(getuid());
    if (pw)
        strncpy(username, pw->pw_name, sizeof(username) - 1);
    //инициализация сокета
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        _ext(-1);
    //разрешение широковещательных пакетов
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broad_perm, sizeof(broad_perm)) < 0)
        _ext(-1);
    //переиспользование порта для множественного запуска приложения на одной машине 
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_perm, sizeof(reuse_perm)) < 0)
        _ext(-1);
    
    //адрес приёма сообщений
    memset(&recvaddr, 0, sizeof(recvaddr));
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_port = htons(IN_PORT);
    recvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //привязка сокета к порту
    if(bind(sockfd, (struct sockaddr*) &recvaddr, sizeof(recvaddr)) < 0)
        _ext(-1);
    
    //удалённый широковещательный адрес 
    memset(&broadaddr, 0, sizeof(broadaddr));
    broadaddr.sin_family = AF_INET;
    broadaddr.sin_port = htons(IN_PORT);
    if(inet_pton(AF_INET, BROADCAST_IP, &broadaddr.sin_addr) == 0) 
        _ext(-1);
    
    char content[CONTENT_LEN];
    snprintf(content, CONTENT_LEN, "%s has joined", username);
    _msg_send(SYS_MSG_HEAD, content);

    fprintf(stdout, "-==Hello-chat==-\n You are %s\n\n", username);
    fflush(stdout);

    child_pid = fork();
    if (child_pid < 0) {
        _ext(-1);
    }

    if (child_pid == 0) {
        //дочерний процесс - слушатель
        msg_t incoming_msg;
        while (1) {
            //бинарные данные структуры из сети
            ssize_t n = recvfrom(sockfd, inbuf, sizeof(msg_t), 0, NULL, NULL);
            if (n > 0) {
                _msg_get(&incoming_msg, inbuf);
                
                if (strcmp(incoming_msg.uname, username) == 0) {
                    continue;
                }
                if (strcmp(incoming_msg.uname, SYS_MSG_HEAD) == 0) {
                    printf("\033[1;33mSystem: %s\033[0m\n", incoming_msg.content);
                } else {
                    printf("\033[1;32m[%s]:\033[0m %s\n", incoming_msg.uname, incoming_msg.content);
                }
                fflush(stdout);
            } else if (n < 0) {
                exit(EXIT_FAILURE);
            }
        }
    } else {
        /* ---------------- РОДИТЕЛЬСКИЙ ПРОЦЕСС: ОТПРАВКА ---------------- */
        char user_input[CONTENT_LEN];

        while (1) {
            if (fgets(user_input, sizeof(user_input), stdin) != NULL) {
                // Удаляем символы переноса строки
                user_input[strcspn(user_input, "\r\n")] = '\0';

                // Обработка ручного выхода из чата
                if (strcmp(user_input, "exit") == 0) {
                    _ext(0); 
                }

                // Пропускаем отправку пустых сообщений
                if (strlen(user_input) == 0) {
                    continue;
                }

                if(_msg_send(username, user_input) < 0)
                    perror("Error sending");
            }
        }
    }

    return 0;
}
