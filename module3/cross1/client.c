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

#include <signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <poll.h>

#include "err.h"
#include "common.h"
#include "htbl.h"

#define CLI_PORT 34400

int sockfd = -1;
char inbuf[CONTENT_LEN], outbuf[CONTENT_LEN];
struct sockaddr_in servaddr, cliaddr;

static void _send_msg(const char* msg) {
    if (sockfd < 0) return;

    uint32_t udp_hdr_len = sizeof(struct udphdr);
    uint32_t msg_len = (uint32_t)strlen(msg);
    uint32_t out_len = udp_hdr_len + msg_len;
    char out_pack[BUFSIZ] = {0};

    //формирование udp заголовка
    struct udphdr *out_udp_hdr = (struct udphdr *)out_pack;
    out_udp_hdr->uh_sport = htons(CLI_PORT);       
    out_udp_hdr->uh_dport = servaddr.sin_port;     
    out_udp_hdr->uh_ulen  = htons((uint16_t)out_len);
    out_udp_hdr->uh_sum   = 0;

    //сам пакет
    memcpy(out_pack + udp_hdr_len, msg, msg_len);

    sendto(sockfd, out_pack, out_len, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
}

static void _ext(int sig){
    if(sig < 0)
        perror(NULL);
    else{
        _send_msg(CMD_EXIT);
        printf("Shut down\n");
    }
    
    if(sockfd > 0)
        close(sockfd);
    
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}



static void _process_packet(char* buffer, ssize_t n){
    const struct iphdr*  ip_hdr  = (const struct iphdr* ) (buffer);
    int ip_hdr_len = ip_hdr->ihl * 4;

    if (n < ip_hdr_len + (ssize_t)sizeof(struct udphdr)) return;

    const struct udphdr* udp_hdr = (const struct udphdr*) (buffer + ip_hdr_len);

    //обработка только тех, что пришли на порт клиента
    if (ntohs(udp_hdr->uh_dport) != CLI_PORT) return;

    uint32_t udp_hdr_len = sizeof(struct udphdr);
    uint32_t data_len = ntohs(udp_hdr->uh_ulen) - udp_hdr_len;
    char* udp_data = (char*)(buffer + ip_hdr_len + udp_hdr_len);

    //от сервера нужны только данные
    char payload[CONTENT_LEN] = {0};
    uint32_t copy_len = (data_len < CONTENT_LEN - 1) ? data_len : CONTENT_LEN - 1;
    memcpy(payload, udp_data, copy_len);
    payload[copy_len] = '\0';

    printf("Server: %s\n", payload);
}

int main(int argc, char* argv[]){
    signal(SIGINT , _ext);
    signal(SIGTERM, _ext);

    if(argc != 2){
        printf("Usage: %s <SERV_IP_ADDR>\n", argv[0]);
        return 0;
    }

    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0){
        _ext(-1);
    }

    //формирование адреса клиента
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
        _ext(-1);

    //формирование адреса сервера
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if(inet_aton(argv[1], &servaddr.sin_addr) == 0) {
        _ext(-1);
    }

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    printf("Connected to server. Type your messages (or type '%s' to exit):\n", CMD_EXIT);

    for (;;) {
        int ret = poll(fds, 2, -1);
        if (ret < 0) {
            if (errno == EINTR) continue; 
            _ext(-1);
        }

        if (fds[0].revents & POLLIN) {
            char input[CONTENT_LEN];
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break; // Конец файла (Ctrl+D)
            }
            input[strcspn(input, "\n")] = 0; // Удаляем перенос строки
            
            if (strlen(input) > 0) {
                if (strcmp(input, CMD_EXIT) == 0) {
                    _ext(0);
                }
                _send_msg(input);
            }
        }

        if (fds[1].revents & POLLIN) {
            char buffer[BUFSIZ];
            struct sockaddr_in src_addr;
            socklen_t addrlen = sizeof(src_addr);
            
            ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addrlen);
            if (n > 0) {
                _process_packet(buffer, n);
            }
        }
    }

    _ext(0);
}