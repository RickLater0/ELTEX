#define _POSIX_C_SOURCE 200809L
#include "tcommon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>

#include <sys/timerfd.h>
#include <time.h>
#include <signal.h>

#define MAX_EVENTS 10
#define NAME_LEN 64

static int rxfd = -1, txfd = -1;
static int tfd  = -1;
static int epfd = -1;

static int current_status = AVALIABLE;
static volatile sig_atomic_t run = 1;
static void _sig_handle(int sig){
    (void) sig;
    run = 0;
}

static void _process_request(msg_t *request){
    switch (request->code)
    {
    case TO_WORK:
    {
        struct itimerspec ts;
        ts.it_value.tv_sec = request->time;
        ts.it_value.tv_nsec = 0;
        ts.it_interval.tv_sec = 0; 
        ts.it_interval.tv_nsec = 0;
            
        timerfd_settime(tfd, 0, &ts, NULL);
        current_status = BUSY;
    }
        break;
    case GET_STATUS:
    {
        msg_t response;
        response.mypid = getpid();
        response.code = current_status;
        if(current_status == BUSY){
            struct itimerspec curr;
            timerfd_gettime(tfd, &curr);
            response.time = (uint32_t)curr.it_value.tv_sec + (uint32_t)(curr.it_value.tv_nsec > 0); 
        }else{
            response.time = 0;
        }
        write(txfd, &response, sizeof(msg_t));
    }
        break;
    default:
        break;
    }
}

int main(void){
    signal(SIGINT , _sig_handle);
    signal(SIGTERM, _sig_handle);
    tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (tfd == -1) {
        perror("timerfd_create");
        return 1;
    }

    int mypid = getpid();
    int ppid = getppid();

    char rx_name[NAME_LEN] = {0};
    char tx_name[NAME_LEN] = {0};
    get_drtx_name(rx_name, NAME_LEN, ppid, mypid);

    get_drrx_name(tx_name, NAME_LEN, ppid, mypid);
    //получение дескрипторов
    
    if((rxfd = open(rx_name, O_RDONLY | O_NONBLOCK)) < 0)
        return 1;
    if((txfd = open(tx_name, O_WRONLY | O_NONBLOCK)) < 0)
        return 1;

    epfd = epoll_create(1);
    struct epoll_event ev;
    
    ev.events = EPOLLIN;
    ev.data.fd = rxfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, rxfd, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

    struct epoll_event events[MAX_EVENTS];
    

    while(run){
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) {
            continue; 
        }
        for(int i = 0; i < n; i++){
            if(events[i].data.fd == rxfd){
                msg_t request;
                if (read(rxfd, &request, sizeof(msg_t)) > 0){
                    _process_request(&request);
                }
            }else if(events[i].data.fd == tfd){
                uint64_t expirations;
                read(tfd, &expirations, sizeof(uint64_t)); //чтение чтобы очистить внутреннести дескриптора
                current_status = AVALIABLE;
            }
        }
    }
    close(epfd);
    close(tfd);
    close(txfd);
    close(rxfd);
}