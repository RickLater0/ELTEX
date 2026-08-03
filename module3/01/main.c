#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>

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



static ssize_t read_all(int fd, void *buf, size_t sz) {
    size_t total = 0;
    char *ptr = (char*)buf;
    while (total < sz) {
        ssize_t r = read(fd, ptr + total, sz - total);
        if (r <= 0) return r; // 0 (EOF) или -1 (ошибка)
        total += (size_t)r;
    }
    return (ssize_t)total;
}

static ssize_t write_all(int fd, const void *buf, size_t sz) {
    size_t total = 0;
    const char *ptr = (const char*)buf;
    while (total < sz) {
        ssize_t w = write(fd, ptr + total, sz - total);
        if (w <= 0) return w; // Ошибка
        total += (size_t)w;
    }
    return (ssize_t)total;
}

static void parent_work(int in, int out, pid_t cpid){
    char msg_buf  [16] ;
    if(out == -1)
        if((out=open(sffname, O_WRONLY)) == -1)
        {
            fprintf(stderr, "UNABLE OUT TO OPEN %s\n", sffname);
            _exit(EXIT_FAILURE);
        }

    if(in == -1)
        if((in=open(ffname, O_RDONLY)) == -1)
        {
            fprintf(stderr, "UNABLE IN TO OPEN %s\n", ffname);
            _exit(EXIT_FAILURE);
        }
    
    for(int i = 0; i < nsz; i++){
        int fd = open(names[i], O_RDONLY);
        if(fd < 0){
            fprintf(stderr, "COULD NOT OPEN THE FILE: %s\n", names[i]);
            continue;
        }

        off_t filesize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        msg_buf[0] = '\0';

        if(read_all(in, msg_buf, MSG_LEN) <= 0 || strncmp(msg_buf, MSG_READY, MSG_LEN)){
            close(fd);
            break;
        }

        write_all(out, MSG_HANDSHAKE, MSG_LEN);

        size_t name_len = strlen(names[i]) + 1;
        write_all(out, &name_len, sizeof(size_t));
        write_all(out, names[i], name_len);
        write_all(out, &filesize, sizeof(off_t));

        char buffer[BUFFER_LEN];
        ssize_t bytes_read;
        while((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            write_all(out, buffer, (size_t)bytes_read);
        }
        close(fd);
    }

    if (read_all(in, msg_buf, MSG_LEN) > 0 && strncmp(msg_buf, MSG_READY, MSG_LEN) == 0) {
        write_all(out, MSG_STOP, MSG_LEN);
    }

    close(in);
    close(out);

    waitpid(cpid, NULL, 0);

    if (ffname != NULL) {
        unlink(ffname);
        unlink(sffname);
    }
}

static void child_work(int in, int out){

    if(in == -1)
        if((in=open(sffname, O_RDONLY)) == -1)
        {
            fprintf(stderr, "UNABLE IN TO OPEN %s\n", sffname);
            _exit(EXIT_FAILURE);
        }
    
    if(out == -1)
        if((out=open(ffname, O_WRONLY)) == -1)
        {
            fprintf(stderr, "UNABLE OUT TO OPEN %s\n", ffname);
            _exit(EXIT_FAILURE);
        }

    while(1){
        write_all(out, MSG_READY, MSG_LEN);

        char cmd[16] = {0};
        // Ожидаем команду
        if (read_all(in, cmd, MSG_LEN) <= 0) break; 

        if (strncmp(cmd, MSG_STOP, MSG_LEN) == 0) {
            break; // Конец работы
        }

        if (strncmp(cmd, MSG_HANDSHAKE, MSG_LEN) == 0) {
            size_t name_len;
            if (read_all(in, &name_len, sizeof(size_t)) <= 0) break;
            
            char fname[512];
            if (read_all(in, fname, name_len) <= 0) break;
            
            off_t filesize;
            if (read_all(in, &filesize, sizeof(off_t)) <= 0) break;

            // Создаем файл с добавлением .copy
            char new_fname[600];
            snprintf(new_fname, sizeof(new_fname), "%s.copy", fname);

            int fd = open(new_fname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                fprintf(stderr, "CHILD CANNOT CREATE FILE: %s\n", new_fname);
                // Мы не можем просто выйти, нужно вычитать присланные данные из pipe
            }

            // Чтение данных файла по частям и их запись
            off_t remaining = filesize;
            char buffer[BUFFER_LEN];
            while (remaining > 0) {
                size_t to_read = (remaining < (off_t)sizeof(buffer)) ? (size_t)remaining : sizeof(buffer);
                ssize_t rb = read_all(in, buffer, to_read);
                if (rb <= 0) break; // Ошибка канала связи

                if (fd >= 0) {
                    write_all(fd, buffer, (size_t) rb);
                }
                remaining -= rb;
            }
            if (fd >= 0) close(fd);
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

    ///неименованные каналы
    int p2c[2] = {-1, -1};//parent to child
    int c2p[2] = {-1, -1};//cild to parent

    ///именнованный канал
    if (ffname) {
        snprintf(sffname, sizeof(sffname), "%s_sync", ffname);
        mkfifo(ffname , 0666);
        mkfifo(sffname, 0666);
    } else {
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
    case 0:
        if(!ffname){
            close(p2c[1]);
            close(c2p[0]);
        }
        child_work(!ffname ? p2c[0] : -1, !ffname ? c2p[1] : -1);
        free(names);
        break;
    default:
        if(!ffname){
            close(c2p[1]);
            close(p2c[0]);
        }
        parent_work(!ffname ? c2p[0] : -1, !ffname ? p2c[1] : -1, pid);
        free(names);
        break;
    }

    return 0;
}

