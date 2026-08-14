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

#include "err.h"
#include "common.h"
#include "htbl.h"

#define LISTEN_QUEUE_SIZE 10 

//хеш-таблица ключ - ip-port объед. в uint64; значение - счётчик 
htbl_s htbl;
int sockfd = -1;
struct sockaddr_in serv_addr, cliaddr;

int clien = sizeof(cliaddr);

static uint32_t _hash_of(const void* key) {
    uint64_t k = *(const uint64_t*)key;
    //хеш - xor верхней и нижней половин
    return (uint32_t)(k ^ (k >> 32));
}

static int _key_equal(const void* k1, const void* k2) {
    uint64_t key1 = *(const uint64_t*)k1;
    uint64_t key2 = *(const uint64_t*)k2;
    return key1 == key2;
}

static void _free_ptr(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

static void _ext(int sig){
    if(sig < 0)
        perror(NULL);
    else{
        printf("Shut down\n");
    }
    
    if(sockfd > 0)
        close(sockfd);
    
    htbl_free(&htbl);
    
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

static uint64_t _make_udp_addr(uint32_t ip_addr, uint32_t udp_port){
    return ((uint64_t)ip_addr << 32) | udp_port;
}

static void _process_packet(char* buffer){

    const struct iphdr*  ip_hdr  = (const struct iphdr* ) (buffer);
    const struct udphdr* udp_hdr = (const struct udphdr*) (buffer  + (ip_hdr->ihl * 4));

    if (ntohs(udp_hdr->uh_dport) != SERV_PORT) {
        return; 
    }

    uint32_t src_ip   = ip_hdr ->saddr   ;
    uint32_t src_port = udp_hdr->uh_sport;

    uint64_t src_addr = _make_udp_addr(src_ip, src_port);

    uint32_t *cli_cnt;

    
    if(htbl_get(&htbl, &src_addr, (void**)&cli_cnt) == HTBL_OK) {
        (*cli_cnt)++;//если такой адрес в таблице есть, счётчик увеличивается
        printf("Client counter %u\n", *cli_cnt);
    } else {//иначе создаётся новая связка ключ-значение 
        uint64_t* new_key = malloc(sizeof(uint64_t));
        if (!new_key) return;
        *new_key = src_addr;
        
        uint32_t* new_cnt = malloc(sizeof(uint32_t));
        if (!new_cnt) { free(new_key); return; }
        *new_cnt = 1;
        
        htbl_put(&htbl, new_key, new_cnt);
        printf("New client 1\n");
        cli_cnt = new_cnt;
    }

    //выборка полезной нагрузки из пакета
    uint32_t udp_hdr_len = sizeof(struct udphdr);
    uint32_t data_len = ntohs(udp_hdr->uh_ulen) - udp_hdr_len;
    char* udp_data = (char*)(buffer + (ip_hdr->ihl * 4) + udp_hdr_len);

    //копирование полезной нагрузки
    char payload[CONTENT_LEN] = {0};
    uint32_t copy_len = (data_len < CONTENT_LEN - 1) ? data_len : CONTENT_LEN - 1;
    memcpy(payload, udp_data, copy_len);
    payload[strcspn(payload, "\r\n")] = 0;

    //команда выхода
    if(strncmp(payload, CMD_EXIT, strlen(CMD_EXIT)) == 0){
        printf("Client exit\n");
        htbl_remove(&htbl, (void*)&src_addr);
        return;
    }

    //создание эхо-сообщения
    char echo[CONTENT_LEN + 32] = {0};
    snprintf(echo, sizeof(echo), "%s %u", payload, *cli_cnt);

    uint32_t out_len = udp_hdr_len + (uint32_t)strlen(echo);
    char out_pack[BUFSIZ] = {0};

    //формирование udp заголовка
    struct udphdr *out_udp_hdr = (struct udphdr *)out_pack;
    out_udp_hdr->uh_sport = udp_hdr->uh_dport; 
    out_udp_hdr->uh_dport = udp_hdr->uh_sport; 
    out_udp_hdr->uh_ulen  = htons((uint16_t)out_len);
    out_udp_hdr->uh_sum   = 0;
    //полный пакет с эхо сообщением
    memcpy(out_pack + udp_hdr_len, echo, strlen(echo));

    //адрес отправителя -> куда сервер отправляет сообщения
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = src_ip;
    dest_addr.sin_port = (in_port_t)src_port;

    if (sendto(sockfd, out_pack, out_len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto failed");
    } else {
        printf("Echo sent: %s\n", echo);
    }
}

int main(int argc, char* argv[]){
    (void) argv;
    if(argc != 1){
        printf("Usage: %s #(no arguments allowed)\n", argv[0]);
        return 0;
    }
    
    signal(SIGINT , _ext);
    signal(SIGTERM, _ext);

    if (htbl_init(&htbl, _hash_of, _key_equal, NULL, _free_ptr, _free_ptr) != HTBL_OK) {
        _ext(-1);
    }

    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0)
        _ext(-1);

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

    char buffer[BUFSIZ];
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(src_addr);
    for(;;){
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addrlen);
        if(n <= 0)
            break;
        _process_packet(buffer);
    }

    _ext(0);
}