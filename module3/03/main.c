#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
/*
Написать программу, использующую две очереди сообщений для
двухстороннего асинхронного взаимодействия (p2p чат). При запуске
программы указывается имя очереди сообщений. Далее к имени добавляется
"_1" для первой очереди и "_2" для второй очереди. Если очереди с такими
именами еще не созданы, процесс и создает их, и через первую очередь
принимает сообщения, а через вторую очередь отправляет сообщения. Если
очереди уже существуют, то первая используется для отправки сообщений, а
вторая – для приема сообщений. Для завершения обмена предусмотреть
отправку данных с заранее известным приоритетом, чтобы уведомить вторую
сторону об окончании работы. Процесс, который создал очереди, должен
удалить их. Предусмотреть отправку сообщения об окончании работы при
получении сигнала SIGINT.
*/

#define ERR_RET -1
#define NOERR 0

//не больше 10 сообщений для обычных пользователей
#define MAXMSG       10
#define CONTENT_SIZE 128
#define NAME_SIZE    32
#define QNAME_SIZE   128

#define PRIO_EXIT    10
#define PRIO_BASIC   1

typedef struct msg
{
    char name[NAME_SIZE];
    char content[CONTENT_SIZE];
} msg_t;

static char char_name[NAME_SIZE] = "Jhon Doe";

static char bq_name[QNAME_SIZE] = {0};
static char q1_name[QNAME_SIZE + 3] = {0}; //3 - "/_1"
static char q2_name[QNAME_SIZE + 3] = {0};

static mqd_t rx_mq = (mqd_t)-1;//recieve
static mqd_t tx_mq = (mqd_t)-1;//transmit

static int is_creator = 0;
static volatile sig_atomic_t running = 1;
static volatile sig_atomic_t msg_flag = 0;

//подписка на уведомления
static void register_notify(void) {
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    sev.sigev_value.sival_ptr = NULL;

    if (mq_notify(rx_mq, &sev) == -1) {
        if (errno != EBUSY) {
            perror("mq_notify");
        }
    }
}

// Обработчик сигнала о приходе нового сообщения в очередь
static void handle_sigusr1(int sig) {
    (void)sig;
    msg_flag = 1;
}

// Обработчик сигнала завершения (Ctrl+C)
static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}
//чтение сообщений из очереди
static void process_messages(){
    msg_t msg;
    unsigned int prio;

    for(;;)
    {
        ssize_t bytes = mq_receive(rx_mq, (char*)&msg, sizeof(msg_t), &prio);
        if (bytes < 0) {
            break; // Сообщений больше нет
        }

        if (prio == PRIO_EXIT || strcmp(msg.content, "EXIT") == 0) {
            printf("\n\n[SYS] CHATTER HAS LEFT\n");
            running = 0;
            break;
        }

        printf("\r[%s]: %s\n> ", msg.name, msg.content);
        fflush(stdout);
    }

    //mq_notify срабатывает один раз (при переходе очереди из пустой в непустую)
    //необходимо обновить подписку на обновления
    if (running) 
        register_notify();
}

//./qchat.elf -n Rick chat.q
//./qchat.elf chat.q

int main(int argc, char* argv[]){

    for(int i = 1; i < argc; i++){
        if(strcmp("-n", argv[i]) == 0 && i + 1 < argc){
            strncpy(char_name, argv[++i], NAME_SIZE );
            char_name[NAME_SIZE  - 1] = '\0';
        }else{
            strncpy(bq_name,    argv[i],  QNAME_SIZE);
            bq_name  [QNAME_SIZE - 1] = '\0';
        }
    }

    if(bq_name[0] == '\0'){
        fprintf(stderr, "NO QUEUE NAME. EXIT\n");
        return -1;
    }

    if (bq_name[0] == '/') {
        snprintf(q1_name, sizeof(q1_name), "%s_1", bq_name);
        snprintf(q2_name, sizeof(q1_name), "%s_2", bq_name);
    } else {
        snprintf(q1_name, sizeof(q1_name), "/%s_1", bq_name);
        snprintf(q2_name, sizeof(q1_name), "/%s_2", bq_name);
    }


    struct mq_attr attr;
    attr.mq_flags = O_NONBLOCK;
    attr.mq_maxmsg = MAXMSG;
    attr.mq_msgsize = sizeof(msg_t);
    attr.mq_curmsgs = 0;
        
    //обработчики сигналов без флага SA_RESTART, 
    //чтобы блокирующие вызовы (fgets) прерывались при получении сигнала
    struct sigaction sa_int, sa_usr;

    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_usr.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr, NULL);

    mqd_t q1 = mq_open(q1_name, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &attr);

    if(q1 != (mqd_t)-1){
        //создатель очереди
        is_creator = 1;

        mqd_t q2 = mq_open(q2_name, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &attr);
        if (q2 == (mqd_t)-1) {
            perror("Q2 CREATION ERR");
            mq_close(q1);
            mq_unlink(q1_name);
            return ERR_RET;
        }

        rx_mq = q1;//получение сообщений через первую очередь
        tx_mq = q2;//отправка сообщений через вторую очередь
        printf("CREATOR: rx: %s, tx: %s\n", q1_name, q2_name);
    }else if (errno == EEXIST){
        //пользователь очереди
        is_creator = 0;

        q1 = mq_open(q1_name, O_RDWR | O_NONBLOCK, 0600, &attr);
        mqd_t q2 = mq_open(q2_name, O_RDWR | O_NONBLOCK, 0600, &attr);
        if (q1 == (mqd_t)-1 || q2 == (mqd_t)-1) {
            perror("Q OPEN ERR");
            mq_close(q1);
            mq_close(q2);
            return ERR_RET;
        }

        rx_mq = q2;//получение сообщений через первую очередь
        tx_mq = q1;//отправка сообщений через вторую очередь
        printf("CLIENT: rx: %s, tx: %s\n", q2_name, q1_name);
    }else{
        perror("Q OPEN ERR");
        return ERR_RET;
    }

    register_notify();

    printf("USR: %s. 'exit' or CTRL+C to leave\n", char_name);
    printf("> ");
    fflush(stdout);

    char msg_buffer[CONTENT_SIZE];

    while(running){

        //проверка наличия сообщений
        if(msg_flag){
            msg_flag = 0;
            process_messages();
        }

        //чтение ввода с клавиатуры
        if(fgets(msg_buffer, sizeof(msg_buffer), stdin) != NULL){
            //замена переноса строки на 0
            msg_buffer[strcspn(msg_buffer, "\n")] = '\0';

            if (strlen(msg_buffer) == 0) {
                printf("> ");
                fflush(stdout);
                continue;
            }

            if (strcmp(msg_buffer, "exit") == 0) {
                running = 0;
                break;
            }

            //компановка сообщения
            msg_t send_msg;
            strncpy(send_msg.name, char_name, NAME_SIZE);
            strncpy(send_msg.content, msg_buffer, CONTENT_SIZE);

            //отправка сообщения
            if (mq_send(tx_mq, (const char*)&send_msg, sizeof(msg_t), PRIO_BASIC) == -1) {
                perror("\nSENDING ERR");
            }

            if (running) {
                printf("> ");
                fflush(stdout);
            }
        }else{
            //fgets вернул NULL. пришёл SIGINT, SIGUSR1 или EOF
            if (errno == EINTR) {
                errno = 0;
                if (msg_flag) {
                    msg_flag = 0;
                    process_messages();
                }
            } else {
                //действительный EOF или ошибка ввода
                break;
            }
        }
    }

    //отпрака сообщения о выходе пользователя
    if (tx_mq != (mqd_t)-1) {
        msg_t exit_msg;
        strncpy(exit_msg.name, "SYSTEM", NAME_SIZE);
        strncpy(exit_msg.content, "EXIT", CONTENT_SIZE);
        mq_send(tx_mq, (const char*)&exit_msg, sizeof(msg_t), PRIO_EXIT);
    }

    //отписка
    mq_notify(rx_mq, NULL);

    ///закрытие ресурсов
    if (rx_mq != (mqd_t)-1) mq_close(rx_mq);
    if (tx_mq != (mqd_t)-1) mq_close(tx_mq);

    // Удаление очередей из системы производит только создатель
    if (is_creator) {
        printf("\n[SYS] CLEARING QUEUES (%s, %s)\n", q1_name, q2_name);
        mq_unlink(q1_name);
        mq_unlink(q2_name);
    }

    printf("\nEND\n");

    return NOERR;
}