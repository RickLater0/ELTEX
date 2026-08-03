#include "Calc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>

/*
Написать программу-калькулятор: в основном меню программа
запрашивает у пользователя необходимое действие и аргументы. Затем
вызывает указанную функцию и выводит результат. После этого снова
запрашивает действие.

Усовершенствовать программу-калькулятор: состав возможных
команд определяется динамически, с использованием указателей на функции.
*/
/*
Доработать решение задачи 2.3 (калькулятор) так, чтобы
функции загружались из динамических библиотек. В одной библиотеке
находится одна функция. При запуске программы считывается каталог с
библиотеками и загружаются найденные функции.
*/

typedef double (*calcfun)(double, double);

typedef struct {
    char name[16];
    calcfun fun;
} Command;

int register_command(Command **commands, int *count, int *capacity, const char *name, calcfun fun) {
    if (*count >= *capacity) {
        *capacity = (*capacity == 0) ? 2 : (*capacity * 2);
        Command *temp = (Command*)realloc(*commands, (*capacity) * sizeof(Command));
        if (!temp) return 0;
        *commands = temp;
    }
    
    strncpy((*commands)[*count].name, name, sizeof((*commands)[0].name) - 1);
    (*commands)[*count].name[sizeof((*commands)[0].name) - 1] = '\0';
    (*commands)[*count].fun = fun;
    (*count)++;
    return 1;
}

int main(){
    int capacity = 0;
    int count = 0;
    Command *commands = NULL;
    const char* dname = "lib";
    DIR* lib = opendir(dname);
    char lname[512];
    char fname[64];
    int handlecount = 0;
    void* handles[16];
    struct dirent* dr = NULL;
    while((dr = readdir(lib)) != NULL){
        int len = strlen(dr->d_name);
        char* ext = dr->d_name + len - 3; 
        if(strcmp(ext, ".so") != 0)
            continue;
        
        sprintf(lname, "%s/%s", dname, dr->d_name);
        void* handle = dlopen(lname, RTLD_NOW);
        if(handle == NULL)
            continue;
        calcfun calc = (calcfun) dlsym(handle, "calc"); 
        if(calc == NULL){
            dlclose(handle);
            continue;
        }
        snprintf(fname, 4, "%3s", dr->d_name + 3);
        register_command(&commands, &count, &capacity, fname, calc);
        handles[handlecount] = handle;
        handlecount++;
    }
    closedir(lib);

    
    
    // Динамическая регистрация базовых команд калькулятора
    
    

    char input[32];
    double a, b;

    printf("=== Динамический консольный калькулятор ===\n");

    while (1) {
        // Вывод динамически сформированного меню
        printf("\nДоступные действия:\n");
        for (int i = 0; i < count; i++) {
            printf("  [%s], ", commands[i].name);
        }
        printf("  [exit] - Выход из программы\n");
        printf("Введите команду: ");

        if (scanf("%31s", input) != 1) {
            break;
        }

        // Проверка на выход
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Поиск выбранной функции в динамическом списке
        calcfun selected_func = NULL;
        for (int i = 0; i < count; i++) {
            if (strcmp(commands[i].name, input) == 0) {
                selected_func = commands[i].fun;
                break;
            }
        }

        if (selected_func == NULL) {
            printf("Неизвестная команда. Пожалуйста, попробуйте снова.\n");
            // Очистка буфера ввода при неверном вводе
            while (getchar() != '\n');
            continue;
        }

        // Запрос аргументов
        printf("Введите два аргумента через пробел: ");
        if (scanf("%lf %lf", &a, &b) != 2) {
            printf("Ошибка: неверный формат аргументов.\n");
            while (getchar() != '\n');
            continue;
        }

        // Вызов функции через указатель и вывод результата
        double result = selected_func(a, b);
        printf("Результат: %.6g\n", result);
    }

    // Освобождение динамически выделенной памяти
    free(commands);
    printf("Работа программы завершена.\n");
    for(;handlecount >= 0;handlecount--){
        dlclose(handles[handlecount]);
    }
    return 0;
}