#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>

#include <signal.h>
#include <sys/wait.h>

#include "tcommon.h"
#include "htbl.h"

#define NAME_LEN 128
#define MAX_PID 65535
#define MAX_EVENTS 22


typedef enum errors{
    T_NOERR = 0,
    T_ARGERR,
    T_MEMERR,
    T_FORKERR,
    T_COLLECTIONERR,
    T_OPENERR,
    T_NOSUCH_DRIVER,
    T_WRITEERR
} TERR;

static htbl_s htbl;
static pid_t last_cpid = -1;
static pid_t taxipid;
static int epfd = -1;

static int _key_eq(const void* k1, const void* k2){
    pid_t key1 = *(const pid_t*)k1;
    pid_t key2 = *(const pid_t*)k2;
    return key1 == key2;
}

static uint32_t _hash_of(const void* key){
    uint32_t x = *(const uint32_t*)key;
    return x * UINT32_C(2654435769);
}

static void _del_ptr(void* ptr){
    free(ptr);
}

static int taxi_init(int inputfd){
    htbl_init(&htbl, _hash_of, _key_eq, NULL, _del_ptr, _del_ptr);
    taxipid = getpid();
    epfd = epoll_create(1);
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = inputfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, inputfd, &ev);
    return T_NOERR;
}

static void _kill_driver(void* key, void* value) {
    pid_t pid = *(pid_t*)key;
    driver_s *dr = (driver_s *)value;
    if (!dr) return;

    kill(pid, SIGINT); 
    
    close(dr->rx);
    close(dr->tx);
    
    char rx_name[NAME_LEN] = {0};
    char tx_name[NAME_LEN] = {0};
    get_drrx_name(rx_name, NAME_LEN, taxipid, pid);
    get_drtx_name(tx_name, NAME_LEN, taxipid, pid);
    
    unlink(rx_name);
    unlink(tx_name);
}

static void _taxi_clear(int sig){
    (void) sig;
    htbl_foreach(&htbl, _kill_driver);
    htbl_free(&htbl);
    close(epfd);
    while(wait(NULL) > 0);
    exit(sig < 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int create_driver(void){

    last_cpid = fork();
    switch (last_cpid)
    {
    case -1:
        perror("fork");
        return T_FORKERR;
        break;
    case 0:
        execl("/media/ELTEX/module3/cross2/target/driver.elf", "driver.elf", (char*)NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    break;
    default:
    {
        //получение уникальных имён для каналов
        char rx_name[NAME_LEN] = {0};
        char tx_name[NAME_LEN] = {0};
        get_drrx_name(rx_name, NAME_LEN, taxipid, last_cpid);
        unlink(rx_name);
        if(mkfifo(rx_name, 0666)){
            perror(NULL);
            kill(last_cpid, SIGKILL);
            return T_OPENERR;
        }
        
        get_drtx_name(tx_name, NAME_LEN, taxipid, last_cpid);
        unlink(tx_name);
        if(mkfifo(tx_name, 0666)){
            perror(NULL);
            kill(last_cpid, SIGKILL);
            return T_OPENERR;
        }
            
        //получение дескрипторов
        int rxfd, txfd;
        if((rxfd = open(rx_name, O_RDONLY | O_NONBLOCK)) < 0){
            return T_OPENERR;
            perror(NULL);
        }
        if((txfd = open(tx_name, O_RDWR| O_NONBLOCK)) < 0){
            perror(NULL);
            
            return T_OPENERR;
        }
        //память под PID водителя
        pid_t* dr_pid = malloc(sizeof(pid_t));
        if(!dr_pid)
            return T_MEMERR;
        *dr_pid = last_cpid;
        //заполнение структуры водителя
        driver_s* dr = malloc(sizeof(driver_s));
        if(!dr){
            free(dr_pid);
            return T_MEMERR;
        }            
        dr->pid = *dr_pid;
        dr->rx = rxfd;
        dr->tx = txfd;

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = dr->rx;
        epoll_ctl(epfd, EPOLL_CTL_ADD, dr->rx, &ev);
        
        if(htbl_put(&htbl, (void*)dr_pid, (void*)dr) != HTBL_OK){
            free(dr_pid);
            free(dr);
            return T_COLLECTIONERR;
        }
        return T_NOERR;
    }
    break;
    }
    return T_NOERR;
}

static int send_task  (pid_t pid, uint32_t task_timer){
    if(pid < 0 || pid > MAX_PID || task_timer == 0)
        return T_ARGERR;
    driver_s* dr;
    int err_code = 0;
    if((err_code = htbl_get(&htbl, &pid, (void**)&dr)) == HTBL_ERR_NOT_FOUND)
        return T_NOSUCH_DRIVER;
    if(err_code != HTBL_OK)
        return T_COLLECTIONERR;
    
    msg_t msg;
    msg.code  = TO_WORK;
    msg.time  = task_timer;
    msg.mypid = taxipid;
    if(write(dr->tx, &msg, sizeof(msg_t)) < 0)
        return T_WRITEERR;
    return T_NOERR;

}

static int get_status (pid_t pid){
    if(pid < 0 || pid > MAX_PID)
        return T_ARGERR;
    driver_s* dr;
    int err_code = 0;
    if((err_code = htbl_get(&htbl, &pid, (void**)&dr)) == HTBL_ERR_NOT_FOUND)
        return T_NOSUCH_DRIVER;
    if(err_code != HTBL_OK)
        return T_COLLECTIONERR;

    msg_t msg;
    msg.code  = GET_STATUS;
    msg.time  = 0;
    msg.mypid = taxipid;

    if(write(dr->tx, &msg, sizeof(msg_t)) < 0)
        return T_WRITEERR;
    return T_NOERR;
}

static void _send_getstat(void* key, void* value){
    (void) key;
    driver_s *dr = (driver_s *)value;
    if (!dr) return;

    msg_t msg;
    msg.code  = GET_STATUS;
    msg.time  = 0;
    msg.mypid = taxipid;
    
    write(dr->tx, &msg, sizeof(msg_t));
}

static int get_drivers(void){
    if (htbl_foreach(&htbl, _send_getstat) != HTBL_OK) {
        return T_COLLECTIONERR;
    }
    return T_NOERR;
}

int main(int argc, char* argv[]){
    if(argc > 1){
        printf("Usage: %s #no arguments allowed\n", argv[0]);
        return 0;
    }

    int inputfd = STDIN_FILENO;
    if (taxi_init(inputfd) != T_NOERR) {
        return 1;
    }
    
    signal(SIGINT , _taxi_clear);
    signal(SIGTERM, _taxi_clear);

    printf("Taxi CLI started. Available commands:\n");
    printf("- create_driver\n- send_task <pid> <timer>\n- get_status <pid>\n- get_drivers\n- exit\n");
    fflush(stdout);

    struct epoll_event events[MAX_EVENTS];
    int run = 1;

    while (run) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == inputfd) {
                char buf[256];
                ssize_t nbytes = read(inputfd, buf, sizeof(buf) - 1);
                
                if (nbytes <= 0) {
                    // Обработка EOF (Ctrl+D) или ошибки чтения
                    run = 0;
                    break;
                }
                
                buf[nbytes] = '\0'; // Нуль-терминируем строку

                char cmd[32];
                pid_t target_pid;
                uint32_t timer;

                // Парсим первое слово как команду
                if (sscanf(buf, "%31s", cmd) == 1) {
                    if (strcmp(cmd, "create_driver") == 0) {
                        if (create_driver() == T_NOERR) {
                            printf("Driver created. PID: %d\n", last_cpid);
                        } else {
                            printf("Error: Failed to create driver.\n");
                        }
                    } 
                    else if (strcmp(cmd, "send_task") == 0) {
                        if (sscanf(buf, "%*s %d %u", &target_pid, &timer) == 2) {
                            int err = send_task(target_pid, timer);
                            if (err != T_NOERR) {
                                printf("Error: send_task failed (code %d).\n", err);
                            } else {
                                printf("Task sent to driver %d for %u seconds.\n", target_pid, timer);
                            }
                        } else {
                            printf("Usage: send_task <pid> <task_timer>\n");
                        }
                    } 
                    else if (strcmp(cmd, "get_status") == 0) {
                        if (sscanf(buf, "%*s %d", &target_pid) == 1) {
                            int err = get_status(target_pid);
                            if (err != T_NOERR) {
                                printf("Error: get_status failed (code %d).\n", err);
                            }
                        } else {
                            printf("Usage: get_status <pid>\n");
                        }
                    } 
                    else if (strcmp(cmd, "get_drivers") == 0) {
                        int err = get_drivers();
                        if (err != T_NOERR) {
                            printf("Error: get_drivers failed (code %d).\n", err);
                        }
                    } 
                    else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
                        run = 0;
                        break;
                    } 
                    else {
                        printf("Unknown command: %s\n", cmd);
                    }
                }
                fflush(stdout);
                
            } else {
                // Обработка ответов от дочерних процессов
                msg_t response;
                if (read(events[i].data.fd, &response, sizeof(msg_t)) > 0) {
                    // Используем поле mypid для идентификации ответившего процесса
                    if (response.code == BUSY) {
                        printf("[Driver %d] Status: Busy (%u sec remaining)\n", response.mypid, response.time);
                    } else if (response.code == AVALIABLE) { 
                        printf("[Driver %d] Status: Available\n", response.mypid);
                    }
                    fflush(stdout);
                }
            }
        }
    }
    
    printf("Shutting down...\n");
    _taxi_clear(0);
    return 0;
}