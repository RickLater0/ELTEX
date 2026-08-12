#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <errno.h>
#include <netinet/udp.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h> 

#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>

#define BUFFER_SIZE 8192
#define NAME_LEN 128
#define ETHERTYPE_IP 0x0800

int sockfd = -1;
FILE* log_file = NULL;
uint32_t pnum = 1;
struct timespec start_time;

// Параметры фильтров
int filter_dns = 0;           // включить фильтр DNS (порт 53)
int filter_task6_port = 0;    // порт для задачи 6 (если 0 – не фильтровать)
int filter_src_port = 0;      // фильтр по порту источника
int filter_dst_port = 0;      // фильтр по порту назначения
char filter_src_ip[16] = "";  // фильтр по IP источника
char filter_dst_ip[16] = "";  // фильтр по IP назначения

static double time_diff_seconds(struct timespec *start, struct timespec *end) {
    return (double)(end->tv_sec - start->tv_sec) + (double)(end->tv_nsec - start->tv_nsec) / 1e9;
}

static void _ext(int sig){
    if(sig < 0)
        perror(NULL);
    if(log_file){
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        fprintf(log_file, "\nCapture ended after %6.6f seconds. %u packets has been logged", time_diff_seconds(&start_time, &now), pnum-1);
        fflush (log_file);
        fclose (log_file);
    }
    if(sockfd > 0){
        close(sockfd);
    }
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int should_log(const struct iphdr *ip_hdr, const struct udphdr *udp_hdr) {
    uint16_t src_port = ntohs(udp_hdr->source);
    uint16_t dst_port = ntohs(udp_hdr->dest);
    char src_ip_str[INET_ADDRSTRLEN];
    char dst_ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_hdr->saddr, src_ip_str, sizeof(src_ip_str));
    inet_ntop(AF_INET, &ip_hdr->daddr, dst_ip_str, sizeof(dst_ip_str));

    // Фильтр DNS (порт 53)
    if (filter_dns) {
        if (src_port != 53 && dst_port != 53)
            return 0;   // не DNS
    }

    // Фильтр для задачи 6 (по заданному порту)
    if (filter_task6_port != 0) {
        if (src_port != filter_task6_port && dst_port != filter_task6_port)
            return 0;
    }

    // Фильтр по порту источника
    if (filter_src_port != 0 && src_port != filter_src_port) return 0;
    // Фильтр по порту назначения
    if (filter_dst_port != 0 && dst_port != filter_dst_port) return 0;
    // Фильтр по IP источника
    if (strlen(filter_src_ip) > 0 && strcmp(src_ip_str, filter_src_ip) != 0) return 0;
    // Фильтр по IP назначения
    if (strlen(filter_dst_ip) > 0 && strcmp(dst_ip_str, filter_dst_ip) != 0) return 0;

    return 1; // все условия выполнены
}

static void watch_packet(const char* buffer){
    const struct ethhdr *eth_hdr = (const struct ethhdr *)buffer;
    
    if (ntohs(eth_hdr->h_proto) != ETHERTYPE_IP)
        return;
    const struct iphdr* ip_hdr = (const struct iphdr*) (buffer + ETH_HLEN);
    if (ip_hdr->protocol != IPPROTO_UDP)
        return;
    const struct udphdr* udp_hdr = (const struct udphdr*) (buffer + ETH_HLEN + (ip_hdr->ihl * 4));

    if(!should_log(ip_hdr, udp_hdr))
        return;

    uint16_t src_port = ntohs(udp_hdr->source);
    uint16_t dst_port = ntohs(udp_hdr->dest  );
    uint16_t udp_len  = ntohs(udp_hdr->len   );

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = time_diff_seconds(&start_time, &now);

    char src_ip_str[INET_ADDRSTRLEN];
    char dst_ip_str[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &ip_hdr->saddr, src_ip_str, sizeof(src_ip_str));
    inet_ntop(AF_INET, &ip_hdr->daddr, dst_ip_str, sizeof(dst_ip_str));

    fprintf(log_file, "%-6u\t%-8.6f\tUDP len=%-8u;\tIP %17s:%-5d -> %17s:%-5d;\n",
        pnum, elapsed, udp_len,
        src_ip_str, src_port,
        dst_ip_str, dst_port
    );
    fflush(log_file);
    pnum++;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "Options:\n"
        "  --dns              Filter only DNS packets (port 53)\n"
        "  --task6 PORT       Filter packets for task6 processes (port PORT)\n"
        "  --src-port PORT    Filter by source port\n"
        "  --dst-port PORT    Filter by destination port\n"
        "  --src-ip IP        Filter by source IP address\n"
        "  --dst-ip IP        Filter by destination IP address\n"
        "  --help             Show this help\n"
        "\n"
        "If no filters are specified, all UDP packets are captured.\n"
        "You can combine filters (AND logic).\n",
        prog);
}

static void set_opt(int argc, char* argv[]){
    static struct option long_options[] = {
        {"dns",      no_argument,       0, 'd'},
        {"task6",    required_argument, 0, 't'},
        {"src-port", required_argument, 0, 's'},
        {"dst-port", required_argument, 0, 'D'},
        {"src-ip",   required_argument, 0, 'S'},
        {"dst-ip",   required_argument, 0, 'I'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "dt:s:D:S:I:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                filter_dns = 1;
                break;
            case 't':
                filter_task6_port = atoi(optarg);
                break;
            case 's':
                filter_src_port = atoi(optarg);
                break;
            case 'D':
                filter_dst_port = atoi(optarg);
                break;
            case 'S':
                strncpy(filter_src_ip, optarg, sizeof(filter_src_ip)-1);
                filter_src_ip[sizeof(filter_src_ip)-1] = '\0';
                break;
            case 'I':
                strncpy(filter_dst_ip, optarg, sizeof(filter_dst_ip)-1);
                filter_dst_ip[sizeof(filter_dst_ip)-1] = '\0';
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
        }
    }
}

int main(int argc, char* argv[]){

    signal(SIGINT , _ext);
    signal(SIGTERM, _ext);

    set_opt(argc, argv);

    if((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
        _ext(-1);

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    char log_fname[NAME_LEN] = "udpget.log";
    log_file = fopen(log_fname, "w");
    if (!log_file)
        _ext(-1);
    //запись даты-времени начала
    time_t t = time(NULL);
    struct tm tm_buf;
    struct tm *tm_info = localtime_r(&t, &tm_buf);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(log_file, "Capture started at %s\n", time_str);
    fflush(log_file);

    char buffer[BUFFER_SIZE];
    struct sockaddr_ll src_addr;
    socklen_t addrlen = sizeof(src_addr);
    for(;;){
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &addrlen);
        if(n < 0)
            break;
        
        watch_packet(buffer);
    }
    _ext(0);
}