#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "lib/darr.h"

#define TOPIC_LEN 64
#define PAYLOAD_LEN 512

#define KEY_FILE "/tmp/media.q"
#define PROJ_ID 130001

#define ACT_PRIO 1

#define CMD_SEND         (char)1
#define CMD_SUBSCRIBE    (char)2
#define CMD_UNSUBSCRIBE  (char)4

static char topic[TOPIC_LEN] = "BASIC TOPIC";

typedef struct msg
{
    char topic  [TOPIC_LEN];
    char payload[PAYLOAD_LEN];
} msg_t;

typedef struct qmsg
{
    unsigned long mtype;
    msg_t msg;
} qmsg_t;

typedef struct act_qmsg
{
    unsigned long mtype;
    char  act;
    pid_t pid;
    msg_t msg;
} act_qmsg_t;

typedef struct sub
{
    pid_t pid;
    char topic[TOPIC_LEN];
} sub_t;

static key_t qkey;
static int   msqid;

static volatile sig_atomic_t running = 1;
static void sig_handler(int sig){
    (void) sig;
    running = 0;
}

static void broker(){

    close(open(KEY_FILE, O_CREAT));
    qkey = ftok(KEY_FILE, PROJ_ID);
    if(qkey == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    msqid = msgget(qkey, IPC_CREAT | IPC_EXCL | 0600);

    if(errno == EEXIST){
        fprintf(stderr, "Queue already exists\n");
        exit(EXIT_FAILURE);
    }

    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    darr subs;
    darr pubs;

    darr_init(&subs, sizeof(sub_t));
    darr_init(&pubs, sizeof(pid_t));

    act_qmsg_t rmsg;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("Broker started\n");
    fflush(stdout);

    while(running){
        ssize_t bytes = msgrcv(msqid, &rmsg, sizeof(act_qmsg_t), ACT_PRIO, 0);
        if (bytes == -1) {
            if (errno == EIDRM || errno == EINTR) {
                //очередь удалена или получен сигнал – завершение
                break;
            }
            perror("msgrcv");
            break; // 
        }
        
        switch(rmsg.act){
            case CMD_SEND:
            {
                int found = 0;
                pid_t tmp;
                for (size_t i = 0; i < d_sz(&pubs); i++) {
                    d_gt(&pubs, i, &tmp);
                    if (tmp == rmsg.pid) { found = 1; break; }
                }
                if (!found) {
                    d_add(&pubs, &rmsg.pid);
                }

                sub_t sub;
                for(size_t i = 0; i < d_sz(&subs); i++){
                    d_gt(&subs, i, &sub);
                    if(strcmp(sub.topic, rmsg.msg.topic) != 0)
                        continue;
                    qmsg_t tmsg;
                    tmsg.mtype = (unsigned long)sub.pid;
                    strcpy(tmsg.msg.topic, rmsg.msg.topic);
                    strcpy(tmsg.msg.payload, rmsg.msg.payload);

                    if(msgsnd(msqid, &tmsg, sizeof(qmsg_t) - sizeof(long), 0) == -1)
                        perror("msgsnd to sub");
                }
            }
            break;
            case CMD_SUBSCRIBE:
            {
                sub_t nsub;
                nsub.pid = rmsg.pid;
                strncpy(nsub.topic, rmsg.msg.topic, TOPIC_LEN - 1);
                nsub.topic[TOPIC_LEN - 1] = '\0';
                d_add(&subs, &nsub);
            }
            break;
            case CMD_UNSUBSCRIBE:
            {
                sub_t dsub;
                dsub.pid = rmsg.pid;
                strncpy(dsub.topic, rmsg.msg.topic, TOPIC_LEN - 1);
                dsub.topic[TOPIC_LEN - 1] = '\0';

                for(size_t i = 0; i < d_sz(&subs);){
                    sub_t csub;
                    d_gt(&subs, i, &csub);
                    if(
                        dsub.pid == csub.pid && 
                        (
                            dsub.topic[0] == '\0' ||
                            strcmp(dsub.topic, csub.topic) == 0
                        )
                    ){
                        d_rm(&subs, i);
                    } else
                        i++;
                }
                
            }
            break;
            default:
                fprintf(stderr, "Unknown action: %d\n", rmsg.act);
            break;
        }
    }

    //уведомление о смерти
    pid_t pid;
    for (size_t i = 0; i < d_sz(&pubs); i++) {
        d_gt(&pubs, i, &pid);
        if (kill(pid, SIGINT) == -1 && errno != ESRCH) {
            perror("kill publisher");
        }
    }
    for (size_t i = 0; i < d_sz(&subs); i++) {
        sub_t sub;
        d_gt(&subs, i, &sub);
        if (kill(sub.pid, SIGINT) == -1 && errno != ESRCH) {
            perror("kill subscriber");
        }
    }

    d_er(&subs);
    d_er(&pubs);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    remove(KEY_FILE);
    printf("\nBroker %d shut down\n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

static void gen_msg(act_qmsg_t* msg){
    static unsigned long counter = 1;

    msg->mtype = ACT_PRIO;
    msg->act = CMD_SEND;
    msg->pid = getpid();

    strncpy(msg->msg.topic, topic, TOPIC_LEN - 1);
    msg->msg.topic[TOPIC_LEN - 1] = '\0';

    snprintf(msg->msg.payload, PAYLOAD_LEN, "Message #%lu from publisher %d", counter++, msg->pid);
}

static void publisher(){
    qkey = ftok(KEY_FILE, PROJ_ID);
    if(qkey == -1){
        fprintf(stderr, "Queue does not exist\n");
        exit(EXIT_FAILURE);
    }

    msqid = msgget(qkey, 0600);
    if(msqid == -1){
        perror("msgget");
        fprintf(stderr, "Queue does not exist or access denied\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    act_qmsg_t msg;

    while(running){
        gen_msg(&msg);

        if(msgsnd(msqid, &msg, sizeof(act_qmsg_t) - sizeof(long), 0) == -1){
            //если очередь была удалена брокером (EIDRM) или мы прерваны сигналом (EINTR)
            if (errno == EIDRM || errno == EINTR) {
                break;
            }//иначе - ошибка
            perror("msgsnd");
            break;
        }

        printf("Publisher [%d] sent:\n\t%s - %s\n", 
            getpid(), msg.msg.topic, msg.msg.payload);
        fflush(stdout);

        sleep(5);
    }

    printf("\nPublisher %d shut down\n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

static void subsciber(){
    qkey = ftok(KEY_FILE, PROJ_ID);
    if(qkey == -1){
        fprintf(stderr, "Queue does not exist\n");
        exit(EXIT_FAILURE);
    }
    msqid = msgget(qkey, 0600);
    if(msqid == -1){
        perror("msgget");
        fprintf(stderr, "Queue does not exist or access denied\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    act_qmsg_t cmd;
    cmd.mtype = ACT_PRIO;
    cmd.act = CMD_SUBSCRIBE;
    cmd.pid = getpid();
    strcpy(cmd.msg.topic, topic);
    cmd.msg.payload[0] = '\0';
    if (msgsnd(msqid, &cmd, sizeof(act_qmsg_t) - sizeof(long), 0) == -1) {
        perror("msgsnd subscribe");
        exit(EXIT_FAILURE);
    }

    qmsg_t rmsg;
    while(running){
        ssize_t bytes = msgrcv(msqid, &rmsg, sizeof(qmsg_t) - sizeof(long), getpid(), 0);

        if (bytes == -1) {
            // Очередь удалена брокером (EIDRM) или получен сигнал завершения (EINTR)
            if (errno == EIDRM || errno == EINTR) {
                break;
            }
            perror("msgrcv subscriber");
            break;
        }

        printf("Subscriber [%d] received: \n\t%s - %s\n",
            getpid(), rmsg.msg.topic, rmsg.msg.payload);
        
        fflush(stdout);
    }

    cmd.mtype = ACT_PRIO;
    cmd.act = CMD_UNSUBSCRIBE;
    cmd.pid = getpid();
    cmd.msg.topic[0] = '\0';
    cmd.msg.payload[0] = '\0';
    msgsnd(msqid, &cmd, sizeof(act_qmsg_t) - sizeof(long), 0);
    printf("\nSubscriber %d shut down\n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    unsigned int flag = 0b0;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-b") == 0){
            flag |= 0b001;
        }
        else if(strcmp(argv[i], "-s") == 0){
            if(i + 1 < argc){
                if(argv[i+1][0] == '-'){
                    fprintf(stderr, "Can use only one flag. USE: media.elf -p <TOPIC>\n");
                    return RET_ERR;
                }else{
                    strncpy(topic, argv[++i], TOPIC_LEN);
                    topic[TOPIC_LEN - 1] = '\0';
                }
            }
            flag |= 0b010;
        }
        else if(strcmp(argv[i], "-p") == 0){
            if(i + 1 < argc){
                if(argv[i+1][0] == '-'){
                    fprintf(stderr, "Can use only one flag. USE: media.elf -p <TOPIC>\n");
                    return RET_ERR;
                }else{
                    strncpy(topic, argv[++i], TOPIC_LEN);
                    topic[TOPIC_LEN - 1] = '\0';
                }
            }
            flag |= 0b100;
        }
    }
    if(flag == 0){
        fprintf(stderr, "At least one flag is needed\n");
        return RET_ERR;
    }
    if((flag & (flag - 1)) != 0){
        fprintf(stderr, "Can use only one flag\n");
        return RET_ERR;
    }

    if(flag == 0b001){
        broker();
    }else if(flag == 0b010){
        subsciber();
    }else if(flag == 0b100){
        publisher();
    }
}