#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define MAXSTR 32

#define NOERR 0
#define ERRPARSE -1

/*
Написать программу, имитирующую принятие решения при
отправке пакета данных по сети с адресацией IPv4. В аргументах запуска
пользователь должен указать IP адрес шлюза, маску подсети и количество
пакетов (N) для имитации обработки. Программа генерирует N случайных
чисел – IP адресов назначения в пакете. Для каждого числа выполняется
преобразование в IP адрес и определяется принадлежность подсети.
После обработки всех пакетов выводится статистика: сколько пакетов (штук
и %) предназначались узлам «своей» подсети, и сколько пакетов
предназначались узлам других сетей.
*/

int8_t parseaddr(const char* const addr, uint32_t* result){
    uint16_t dec = 0;
    uint32_t bt = 0;
    uint8_t dots = 0;
    char n[4];
    //printf("parse begin\n");
    const char* c = addr;
    #if 0
    for(; *c != '\0' && *c != ' ' && *c != '\n';){
        int i = 0;
        for(; *c != '.' && *c!= '\0'; c++){
            if(*c == '\0')
                break;
            if(i < 3){
                if(*c >= '0' && *c <= '9')
                    n[i++] = *c;
                else
                    return ERRPARSE;
            }else{
                printf("err parsed i - str: %d - %s\n", i, n);
                return ERRPARSE;
            }
        }
        n[i] = '\0';
        printf("parsed i - str: %d - %s\n", i, n);
        if(n[0] >= '0' && n[0] <= '9')
            dec = atoi(n);
        else
            dec = 0;
        if(dec > 255)
            return ERRPARSE;
        bt |= dec << ((3 - dots) * 8);
        if(dots == 3)
            break;
        dots++;
    }
    #endif

    while(*c != '\0' && *c != ' ' && *c != '\n'){
        int i = 0;
        while(*c != '.' && *c != '\0' && *c != ' ' && *c != '\n'){
            if(i < 3){
                if(*c >= '0' && *c <= '9')
                    n[i++] = *c;
                else
                    return ERRPARSE;
            }else{
                //printf("err parsed i - str: %d - %s\n", i, n);
                return ERRPARSE;
            }
            c++;
        }

        n[i] = '\0';
        //printf("parsed i - str: %d - %s\n", i, n);

        if(n[0] >= '0' && n[0] <= '9')
            dec = atoi(n);
        else
            dec = 0;

        if(dec > 255)
            return ERRPARSE;
        bt |= dec << ((3 - dots) * 8);

        if(*c == '.'){
            c++;
            dots++;
        }
    }

    if(dots == 0 && dec <= 32){
        bt = 0;
        for(int i = 0; i < dec; i++){
            bt |= 0x80000000 >> i;
        }
    }else if(dots != 3){
        //printf("dots != 3");
        return ERRPARSE;
    }
    *result = bt;
    #if 0
    printf("%u.%u.%u.%u\n\n",
        (uint8_t)(bt >> 24), 
        (uint8_t)(bt >> 16), 
        (uint8_t)(bt >>  8), 
        (uint8_t)(bt >>  0)
    );
    #endif
    return NOERR;
}

int8_t test
(
    const uint32_t gateway, 
    const uint32_t mask,
    const uint32_t out
){
    uint32_t targetnet = gateway & mask;
    uint32_t undermask = out & mask;

    uint32_t fin = targetnet ^ undermask;

    if(fin > 0) return 0;
    else return 1;
}

uint32_t random_ip() {
    return ((uint32_t)(rand() % 256) << 24) |
           ((uint32_t)(rand() % 256) << 16) |
           ((uint32_t)(rand() % 256) << 8)  |
           ((uint32_t)(rand() % 256));
}

int main(int argc, char *argv[]){
    if (argc != 4) {
        fprintf(stderr, "%s <IPv4 шлюза> <маска> <число тестов>\n", argv[0]);
        return 1;
    }

    const char *gateway_str = argv[1];
    const char *mask_str = argv[2];
    if(argv[3][0]=='-'){
        fprintf(stderr, "N обязано быть положительным\n");
        return 1;
    }
    uint64_t N = strtoull(argv[3], NULL, 10);

    uint32_t gateway = 0;
    uint32_t mask = 0;
    if(parseaddr(gateway_str, &gateway) == ERRPARSE){
        printf("Ошибка парсинга шлюза\n");
        return 1;
    }
    if(parseaddr(mask_str, &mask) == ERRPARSE){
        printf("Ошибка парсинга маски\n");
        return 1;
    }
    
    uint64_t same_subnet = 0;
    srand(time(NULL));
    
    for (uint64_t i = 0; i < N; i++) {
        uint32_t dest_ip = random_ip();
        if(test(gateway, mask, dest_ip)){
            same_subnet++;
        }
    }
    printf("Нашим: %lu/%lu -> %.6f%%\n", same_subnet, N, ((double)same_subnet/N)*100.);
    printf("Вашим: %lu/%lu -> %.6f%%\n", N - same_subnet, N, ((double)(N -same_subnet)/N)*100.);
    return 0;
}