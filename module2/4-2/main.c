#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Вспомогательная функция для генерации текстового сообщения
char* create_message(const char* text) {
    char* msg = (char*)malloc(strlen(text) + 1);
    if (msg) {
        strcpy(msg, text);
    }
    return msg;
}

// Автоматическая имитация работы очереди
void run_simulation() {
    printf("=== СИМУЛЯЦИЯ АВТОМАТИЧЕСКОЙ ГЕНЕРАЦИИ И ВЫБОРКИ ===\n");

    pqueue* pq;
    if (mkq(&pq) < NOERR) {
        printf("Ошибка создания очереди в симуляции.\n");
        free(pq);
        return;
    }

    printf("1. Добавление сообщений с разными приоритетами...\n");
    // Приоритет 0 — наивысший, 255 — наинизший
    enq(pq, create_message("Критическая ошибка системы"), 0);
    enq(pq, create_message("Низкоприоритетный лог"), 200);
    enq(pq, create_message("Важное уведомление"), 50);
    enq(pq, create_message("Стандартный warning"), 100);
    enq(pq, create_message("Еще одно важное уведомление"), 50);

    void* extracted_data = NULL;

    char* q_string = NULL;
    ERR_TYPE res = pqtostr(pq, &q_string);

    if (res >= NOERR && q_string != NULL) {
        printf("Текущее состояние очереди:\n%s\n", q_string);
        
        free(q_string); 
    } else {
        printf("Ошибка при генерации строки! Код: %d\n", res);
    }

    // Тест 1: Извлечение с точным приоритетом (pdeq)
    printf("\n2. Тест pdeq (ищем точный приоритет 100):\n");
    if (pdeq(pq, &extracted_data, 100) >= NOERR) {
        printf("Извлечено: %s (осталось элементов: %u)\n", (char*)extracted_data, pqsz(pq));
        free(extracted_data);
    } else {
        printf("Элемент с приоритетом 100 не найден.\n");
    }

    // Тест 2: Извлечение с приоритетом не ниже заданного (spdeq)
    // "Не ниже" в контексте данной логики (где 0 — лучший) означает <= по значению
    printf("\n3. Тест spdeq (ищем приоритет не ниже/хуже 60, т.е. число >= 60):\n");
    if (spdeq(pq, &extracted_data, 60) >= NOERR) {
        printf("Извлечено: %s (осталось элементов: %u)\n", (char*)extracted_data, pqsz(pq));
        free(extracted_data);
    } else {
        printf("Элементы с приоритетом >= 60 не найдены.\n");
    }

    // Тест 3: Извлечение первого доступного из очереди (deq)
    printf("\n4. Тест deq (извлечение первого/последнего элемента согласно логике remove):\n");
    while (pqsz(pq) > 0) {
        deq(pq, &extracted_data);
        printf("Извлечено через deq: %s\n", (char*)extracted_data);
        free(extracted_data);
    }

    printf("\nСимуляция успешно завершена.\n");
    frq(pq);
    printf("===================================================\n\n");
}

int main(){

    run_simulation();

    pqueue* pq;
    if(mkq(&pq) < 0){
        printf("Не удалось инициализировать очередь для меню.\n");
        free(pq);
        return 1;
    }

    int choice = 0;
    printf("Добро пожаловать в интерактивное тестирование очереди!\n");

    while (1) {
        printf("\n--- МЕНЮ УПРАВЛЕНИЯ ОЧЕРЕДЬЮ (Текущий размер: %u) ---\n", pqsz(pq));
        printf("1. Добавить элемент (enq)\n");
        printf("2. Извлечь первый элемент (deq)\n");
        printf("3. Извлечь с точным приоритетом (pdeq)\n");
        printf("4. Извлечь с приоритетом не ниже заданного (spdeq)\n");
        printf("5. Отобразить очередь\n");
        printf("0. Выйти из программы\n");
        printf("Выберите действие: ");

        if (scanf("%d", &choice) != 1) {
            printf("Некорректный ввод! Используйте числа.\n");
            clear_input();
            continue;
        }
        clear_input();

        if (choice == 0) {
            printf("Выход из программы и освобождение памяти.\n");
            break;
        }

        void* data_ptr = NULL;
        ERR_TYPE result = NOERR;
        PRIO_TYPE prio_input = 0;
        char buffer[256];

        switch (choice) {
            case 1:
            {
                
                printf("Введите текст сообщения: ");
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    // Удаляем символ переноса строки, если он есть
                    buffer[strcspn(buffer, "\n")] = '\0';
                }
                
                printf("Введите приоритет (0 - %d): ", LOWEST_PRIO);
                int temp_prio;
                if (scanf("%d", &temp_prio) != 1 || temp_prio < 0 || temp_prio > 255) {
                    printf("Неверный приоритет! Должен быть от 0 до 255.\n");
                    clear_input();
                    break;
                }
                clear_input();
                
                prio_input = (PRIO_TYPE)temp_prio;
                char* new_msg = create_message(buffer);
                result = enq(pq, new_msg, prio_input);
                
                if (result >= 0) {
                    printf("Успешно добавлено.\n");
                } else {
                    printf("Ошибка добавления! Код ошибки: %d\n", result);
                    free(new_msg);
                }
            }
                break;

            case 2:
                result = deq(pq, &data_ptr);
                if (result >= NOERR && data_ptr != NULL) {
                    printf("Успешно извлечен первый элемент: %s\n", (char*)data_ptr);
                    free(data_ptr);
                } else if (result == NOT_FOUND) {
                    printf("Очередь пуста!\n");
                } else {
                    printf("Ошибка извлечения! Код: %d\n", result);
                }
                break;

            case 3:
            {
                printf("Введите точный приоритет для поиска: ");
                int exact_prio;
                if (scanf("%d", &exact_prio) != 1) {
                    printf("Некорректный ввод.\n");
                    clear_input();
                    break;
                }
                clear_input();

                result = pdeq(pq, &data_ptr, (PRIO_TYPE)exact_prio);
                if (result >= NOERR && data_ptr != NULL) {
                    printf("Найден и извлечен элемент: %s\n", (char*)data_ptr);
                    free(data_ptr);
                } else {
                    printf("Элемент с таким приоритетом не найден (Код: %d).\n", result);
                }
            }
                break;

            case 4:
            {
                printf("Введите пороговый приоритет (не ниже указанного): ");
                int limit_prio;
                if (scanf("%d", &limit_prio) != 1) {
                    printf("Некорректный ввод.\n");
                    clear_input();
                    break;
                }
                clear_input();

                result = spdeq(pq, &data_ptr, (PRIO_TYPE)limit_prio);
                if (result >= NOERR && data_ptr != NULL) {
                    printf("Найден и извлечен элемент: %s\n", (char*)data_ptr);
                    free(data_ptr);
                } else {
                    printf("Подходящий элемент не найден (Код: %d).\n", result);
                }
            }
                break;
            case 5:
            {
                char* q_string = NULL;
                ERR_TYPE res = pqtostr(pq, &q_string);

                if (res >= NOERR && q_string != NULL) {
                    printf("Текущее состояние очереди:\n%s\n", q_string);
                    
                    free(q_string); 
                } else {
                    printf("Ошибка при генерации строки! Код: %d\n", res);
                }
            }
                break;
            default:
                printf("Неверный пункт меню. Попробуйте снова.\n");
                break;
        }
    }

    frq(pq);
    return 0;
}