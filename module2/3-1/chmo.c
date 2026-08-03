#include "chmo.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

uint16_t r = 0444;
uint16_t w = 0222;
uint16_t x = 0111;

uint16_t u = 0700;
uint16_t g = 0070;
uint16_t o = 0007;

//chmo ~/testfile 777
//set rights for file to 777
//chmo ~/testfile ug=rx,o=

/*
Написать программу для расчета маски прав доступа к файлу.
1)Пользователь может ввести права доступа в буквенном или цифровом
обозначении, ему должно быть показано соответствующее битовое
представление.

2)Пользователь может ввести имя файла, и ему отобразится буквенное,
цифровое и битовое представление прав доступа. Использовать функцию stat
для получения информации о файле. Сравнить результат с выводом,
например, ls -l.

3)Пользователь может изменить права доступа, определенные в первом или
втором пункте, введя команды модификации атрибутов (подобно команде
chmod). При этом отображается буквенное, цифровое и битовое
представление прав доступа. Изменение прав доступа не нужно применять к
файлу
*/

file files[FMAX];
int fsz = 0;
int8_t chmo(char* fname, char* args, file* farg){
    
    int fin = -1;
    for(int i = 0; i < fsz; i++){
        if(strcmp(fname, files[i].name) == 0){
            fin = i;
            break;
        }
    }
    if(fsz == FMAX){
        return OVERLOAD;
    }
    file f;
    if(fin == -1){
        struct stat st; 
        if(stat(fname, &st) == 0){
            f.rights = st.st_mode & 0777;
        }else{
            f.rights = 0666;
        }
        
        strncpy(f.name, fname, STRMAX);
        f.name[STRMAX - 1] = '\0';
        files[fsz] = f;
        fin = fsz;
        fsz++;
    }else{
        f = files[fin];
    }

    if(*args == '\0'){
        *farg = f;
        return NOARG;
    }
    if(*args >= '0' && *args <= '7'){
        uint16_t mask = 0000;
        char c = *args;
        int k = 0;
        for(int i = 0100; i > 0; i/=010){
            mask |=  i * (c - '0');
            k++;
            c = *(args + k);
            if(c == '\0')
                break;    
        }
        f.rights = mask;
    }else if(
        *args == 'u' || 
        *args == 'g' || 
        *args == 'o' || 
        *args == 'a' ||
        *args == '=' ||
        *args == '-' ||
        *args == '+'
    ){
        uint16_t finmask = f.rights;
        uint16_t umask = 0000;
        uint16_t rmask = 0000;
        for(char* c = args; *c != '\0'; c++){
            umask = 0000;
            rmask = 0000;
            for(;
                *c == 'u' || *c == 'g' ||
                *c == 'o' || *c == 'a'
                ; c++){
                switch (*c)
                {
                case 'u':
                    umask |= u;
                    break;
                case 'g':
                    umask |= g;
                    break;
                case 'o':
                    umask |= o;
                    break;
                case 'a':
                    umask |= (u | g | o);
                    break;
                }
            }
            if(umask == 0)
                umask |= (u | g | o);
            char op = *c;
            c++;
            for(; *c == 'r' || *c == 'w' || *c == 'x'; c++){
                switch (*c)
                {
                case 'r':
                    rmask |= r;
                    break;
                case 'w':
                    rmask |= w;
                    break;
                case 'x':
                    rmask |= x;
                    break;
                }
            }
            switch (op)
            {
            case '+':
                finmask |= (rmask & umask);
                break;
            case '-':
                finmask &= ~(rmask & umask);
                break;
            case '=':
                finmask = (finmask & ~umask) | (rmask & umask);
                break;
            default:
                return ARGERR;
            }
            if(*c == ',')
                continue;
            else
                break;
        }
        f.rights = finmask;
    }else{
        return ARGERR;
    }
    files[fin] = f;
    *farg = f;
    return NOERR;
}
