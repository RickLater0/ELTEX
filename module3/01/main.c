#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>

#define BUFFER_LEN 128

#define MSG_LEN 12

static const char* const MSG_READY      = "READY1    \n";
static const char* const MSG_HANDSHAKE  = "HANDSHAKE1\n";
static const char* const MSG_STOP       = "STOP NOW1 \n";

//fifo names
static char* ffname = NULL;
static char  sffname [128];

static char** names = NULL;
static int    nsz   = 0   ;

//atomic read 
///читает из fd ровно sz байт и записывает в buf. возвращает ошибку (-1, 0) при неудачном чтении
static ssize_t read_all(int fd, void *buf, size_t sz) {
    size_t total = 0;
    char *ptr = (char*)buf;
    while (total < sz) {
        ssize_t r = read(fd, ptr + total, sz - total);
        if (r <= 0) return r; 
        total += (size_t)r;
    }
    return (ssize_t)total;
}

//atomic 
///пишет в fd ровно sz байт из  буфера buf. возвращает ошибку (-1, 0) при неудачной записи
static ssize_t write_all(int fd, const void *buf, size_t sz) {
    size_t total = 0;
    const char *ptr = (const char*)buf;
    while (total < sz) {
        ssize_t w = write(fd, ptr + total, sz - total);
        if (w <= 0) return w; 
        total += (size_t)w;
    }
    return (ssize_t)total;
}
//родительский процесс
static void parent_work(int in, int out, pid_t cpid){
    char msg_buf  [16] ;
    if(out == -1)//"поток" записи
        if((out=open(sffname, O_WRONLY)) == -1)
        {
            fprintf(stderr, "UNABLE OUT TO OPEN %s\n", sffname);
            free(names);
            _exit(EXIT_FAILURE);
        }

    if(in == -1)//"поток" чтения
        if((in=open(ffname, O_RDONLY)) == -1)
        {
            fprintf(stderr, "UNABLE IN TO OPEN %s\n", ffname);
            free(names);
            _exit(EXIT_FAILURE);
        }
    //пока есть файлы
    for(int i = 0; i < nsz; i++){
        //попытка открыть файл
        int fd = open(names[i], O_RDONLY);
        ///TODO: добавить обработку ошибки
        if(fd < 0){//не получилось - к следующим файлам
            fprintf(stderr, "COULD NOT OPEN THE FILE: %s\n", names[i]);
            continue;
        }
        //получение размера файла
        off_t filesize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        msg_buf[0] = '\0';
        //получение от дочернего процесса сообщения о готовности
        if(read_all(in, msg_buf, MSG_LEN) <= 0 || strncmp(msg_buf, MSG_READY, MSG_LEN)){
            close(fd);//иначе - конец процесса 
            break;
        }

        //отправление рукопожатия
        write_all(out, MSG_HANDSHAKE, MSG_LEN);

        //отправка метаинформации о файле
        size_t name_len = strlen(names[i]) + 1;
        write_all(out, &name_len, sizeof(size_t));
        write_all(out, names[i], name_len);
        write_all(out, &filesize, sizeof(off_t));

        char buffer[BUFFER_LEN];
        ssize_t bytes_read;
        //пока есть файл отправление дочернему процессу содержимого файла
        while((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            write_all(out, buffer, (size_t)bytes_read);
        }
        close(fd);
        //переход к следующему файлу
    }
    //файлы закончилисть - отправка дочернему процессу сообщения об остановке
    if (read_all(in, msg_buf, MSG_LEN) > 0 && strncmp(msg_buf, MSG_READY, MSG_LEN) == 0) {
        write_all(out, MSG_STOP, MSG_LEN);
    }
    
    //закрытие ресурсов 
    close(in);
    close(out);
    if (ffname != NULL) {
        unlink(ffname);
        unlink(sffname);
    }

    //ожидание закрытия дочернего процесса
    waitpid(cpid, NULL, 0);
}

static void child_work(int in, int out){

    if(in == -1) //"поток" чтения
        if((in=open(sffname, O_RDONLY)) == -1)
        {
            fprintf(stderr, "UNABLE IN TO OPEN %s\n", sffname);
            _exit(EXIT_FAILURE);
        }
    
    if(out == -1) //"поток" записи
        if((out=open(ffname, O_WRONLY)) == -1)
        {
            fprintf(stderr, "UNABLE OUT TO OPEN %s\n", ffname);
            _exit(EXIT_FAILURE);
        }

    while(1){
        // новый файл - отправка родительскому процессу сообщения о готовности
        write_all(out, MSG_READY, MSG_LEN);

        char cmd[16] = {0};
        // одидание команды
        if (read_all(in, cmd, MSG_LEN) <= 0) break; 

        if (strncmp(cmd, MSG_STOP, MSG_LEN) == 0) {
            break; // сообщение об останова
        }

        // новое рукопожатие - новый файл
        if (strncmp(cmd, MSG_HANDSHAKE, MSG_LEN) == 0) {
            //получение метаинформации
            size_t name_len;
            if (read_all(in, &name_len, sizeof(size_t)) <= 0) break;
            
            char fname[512];   
            if (read_all(in, fname, name_len) <= 0) break;
            
            off_t filesize;
            if (read_all(in, &filesize, sizeof(off_t)) <= 0) break;

            // создание файла *.copy
            char new_fname[600];
            snprintf(new_fname, sizeof(new_fname), "%s.copy", fname);

            ///TODO: добавить обработку ошибки
            int fd = open(new_fname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                fprintf(stderr, "CHILD CANNOT CREATE FILE: %s\n", new_fname);
            }else{
                off_t remaining = filesize;
                char buffer[BUFFER_LEN];
                // пока не кончится файл
                while (remaining > 0) {
                    // чтение данных от родителя 
                    size_t to_read = (remaining < (off_t)sizeof(buffer)) ? (size_t)remaining : sizeof(buffer);
                    ssize_t rb = read_all(in, buffer, to_read);
                    if (rb <= 0) break; 
                    // и запись в файл
                    if (fd >= 0) {
                        write_all(fd, buffer, (size_t) rb);
                    }
                    remaining -= rb;
                }

                close(fd);
            }
        }
    }

    close(in);
    close(out);
}


int main(int argc, char *argv[]){

    names = malloc((unsigned long)argc * sizeof(char*));
    
    for(int i = 1; i < argc; i++){
        if(strcmp("-p", argv[i]) == 0 && i + 1 < argc)
            ffname = argv[++i];
        else
            names[nsz++] = argv[i];
    }

    if(nsz == 0){
        fprintf(stdout, "NO ARGS\n");
        free(names);
        exit(EXIT_SUCCESS);
    }

    //неименованные каналы
    int p2c[2] = {-1, -1};//parent to child
    int c2p[2] = {-1, -1};//cild to parent

    
    if (ffname) {//создание именнованных каналов
        snprintf(sffname, sizeof(sffname), "%s_sync", ffname);
        if(mkfifo(ffname , 0666) != 0){
            fprintf(stderr, "UNABLE TO CREATE FIFO (%s): %s\n", ffname, strerror(errno));
            free(names);
            exit(EXIT_FAILURE);
        }
        
        if(mkfifo(sffname, 0666) != 0){
            fprintf(stderr, "UNABLE TO CREATE FIFO (%s): %s\n", sffname, strerror(errno));
            unlink(ffname);
            free(names);
            exit(EXIT_FAILURE);
        }
    } else {    //создание неименованных каналов
        if (pipe(p2c) == -1 || pipe(c2p) == -1) {
            fprintf(stderr, "CAN NOT CREATE PIPE\n");
            free(names);
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid;
    switch (pid = fork())
    {
    case -1:
        fprintf(stderr, "FORK ERR\n");
        exit(EXIT_FAILURE);
        break;
    case 0://дочерний процесс
        if(!ffname){//закрытие дескрипторов
            close(p2c[1]);
            close(c2p[0]);
        }
        child_work(!ffname ? p2c[0] : -1, !ffname ? c2p[1] : -1);
        free(names);
        break;
    default://родительский процесс
        if(!ffname){//закрытие дескрипторов
            close(c2p[1]);
            close(p2c[0]);
        }
        parent_work(!ffname ? c2p[0] : -1, !ffname ? p2c[1] : -1, pid);
        free(names);
        break;
    }

    return 0;
}

