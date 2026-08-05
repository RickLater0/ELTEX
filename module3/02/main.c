#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define TOPIC_LEN 64
#define PAYLOAD_LEN 512
#define SYS_MSG_LEN 16

static const char q_name[32] = "/tmp/media.q";

typedef struct msg
{
    char topic  [TOPIC_LEN];
    char payload[PAYLOAD_LEN];
} msg_t;

typedef struct act_msg
{
    char content[SYS_MSG_LEN];
    pid_t pid;
    msg_t msg;
} act_msg_t;

typedef struct sub
{
    pid_t pid;
    char topic[TOPIC_LEN];
} sub_t;


static void brocker(){
    sub_t* subs;
    pid_t* pubs;


}

static void publisher(){
    
}

static void subsciber(){

}

int main(int argc, char* argv[]){
    int flag = 0b0;
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-b")){

        }
        else if(strcmp(argv[i], "-s")){
            
        }
        else if(strcmp(argv[i], "-p")){

        }
    }
}