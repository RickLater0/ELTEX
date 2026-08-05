#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

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
    sub_t sub_entry;
    pid_t pub_pid;

    for(;;){
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
                for(int i = 0; i < d_sz(&subs); i++){
                    d_gt(&subs, i, &sub);
                    if(strcmp(sub.topic, rmsg.msg.topic) != 0)
                        continue;
                    qmsg_t tmsg;
                    tmsg.mtype = sub.pid;
                    strcpy(tmsg.msg.topic, rmsg.msg.topic);
                    strcpy(tmsg.msg.payload, rmsg.msg.payload);

                    if(msgsnd(msqid, &tmsg, sizeof(qmsg_t), 0) == -1)
                        perror("msgsnd to sub");
                }
            break;
            case CMD_SUBSCRIBE:
                sub_t sub;
                sub.pid = rmsg.pid;
                strcpy(sub.topic, rmsg.msg.topic);
                d_add(&subs, &sub);
            break;
            case CMD_UNSUBSCRIBE:

            break;
        }
    }


    d_er(&subs);
    d_er(&pubs);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    remove(KEY_FILE);
    exit(EXIT_SUCCESS);
}

static void publisher(){
    qkey = ftok(KEY_FILE, PROJ_ID);
    if(qkey == -1){
        fprintf(stderr, "Queue does not exist\n");
        exit(EXIT_FAILURE);
    }
}

static void subsciber(){
    qkey = ftok(KEY_FILE, PROJ_ID);
    if(qkey == -1){
        fprintf(stderr, "Queue does not exist\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]){
    unsigned int flag = 0b0;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-b")){
            flag |= 0b001;
        }
        else if(strcmp(argv[i], "-s")){
            flag |= 0b010;
        }
        else if(strcmp(argv[i], "-p")){
            if(i + 1 < argc){
                if(argv[i][0] == '-'){
                    fprintf(stderr, "Can use only one flag. USE: media.elf -p <TOPIC>\n");
                    return RET_ERR;
                }else{
                    strncpy(topic, argv[++i], TOPIC_LEN);
                    topic[TOPIC_LEN - 1] = '\0';
                }
            }
            flag |= 0b100;
        }
        if(flag == 0){
            fprintf(stderr, "At least one flag is needed\n");
            return RET_ERR;
        }
        if(flag & (flag - 1) == 0){
            fprintf(stderr, "Can use only one flag\n");
            return RET_ERR;
        }

        
        if(flag == 0b1){
            broker();
        }else if(flag == 0b01){
            subsciber();
        }else{
            publisher();
        }
    }
}