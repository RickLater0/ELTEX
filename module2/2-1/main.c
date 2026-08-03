#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Contacts.h"
#include "lib/Storage.h"

// Прототип searchPerson
extern uint32_t* searchPerson(const Contacts* const contacts, uint32_t* retsz, uint8_t n, ...);

// =========================================================
// СЕРВИСНЫЕ ФУНКЦИИ ВВОДА
// =========================================================

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void trimNewline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
}

void safeInput(char* buffer, size_t size, const char* prompt) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin)) {
        trimNewline(buffer);
    } else {
        buffer[0] = '\0';
    }
}

int safeInputInt(const char* prompt, int* value) {
    printf("%s", prompt);
    if (scanf("%d", value) != 1) {
        clearInput();
        printf("[!] Ошибка: Введено не число.\n");
        return 0;
    }
    clearInput();
    return 1;
}

int safeInputUint32(const char* prompt, uint32_t* value) {
    printf("%s", prompt);
    if (scanf("%u", value) != 1) {
        clearInput();
        printf("[!] Ошибка: Неверный формат числа.\n");
        return 0;
    }
    clearInput();
    return 1;
}

int safeInputUint16(const char* prompt, uint16_t* value) {
    printf("%s", prompt);
    if (scanf("%hu", value) != 1) {
        clearInput();
        printf("[!] Ошибка: Неверный формат числа.\n");
        return 0;
    }
    clearInput();
    return 1;
}

// =========================================================
// ФУНКЦИИ ВВОДА ДЛЯ СОЗДАНИЯ КОНТАКТА (СТРОГИЕ)
// =========================================================

void inputName(Name* name) {
    char buffer[FN_LEN];
    do {
        safeInput(buffer, sizeof(buffer), "Введите имя (обязательно): ");
    } while (strlen(buffer) == 0);
    strncpy(name->firstname, buffer, FN_LEN - 1);
    name->firstname[FN_LEN - 1] = '\0';

    do {
        safeInput(buffer, sizeof(buffer), "Введите фамилию (обязательно): ");
    } while (strlen(buffer) == 0);
    strncpy(name->lastname, buffer, LN_LEN - 1);
    name->lastname[LN_LEN - 1] = '\0';

    safeInput(buffer, sizeof(buffer), "Введите отчество (Enter - пропустить): ");
    strncpy(name->surname, buffer, SN_LEN - 1);
    name->surname[SN_LEN - 1] = '\0';
}

int inputPhones(Phone* phones) {
    int count = 0;
    printf("--- Ввод телефонов (для завершения введите 0 в коде региона) ---\n");
    while (count < PHONES_AMOUNT) {
        uint16_t region = 0;
        safeInputUint16("  Код региона (0 - выход): ", &region);
        if (region == 0) break;
        
        uint32_t number = 0;
        safeInputUint32("  Номер: ", &number);

        phones[count].regionCode = region;
        phones[count].number = number;
        count++;
    }
    return count;
}

int inputEmails(Email* emails) {
    int count = 0;
    printf("--- Ввод email (пустая локальная часть для завершения) ---\n");
    while (count < EMAILS_AMOUNT) {
        char buffer[64];
        safeInput(buffer, sizeof(buffer), "  Локальная часть (до @): ");
        if (strlen(buffer) == 0) break;
        strncpy(emails[count].name, buffer, NAME_LEN - 1);
        emails[count].name[NAME_LEN - 1] = '\0';

        safeInput(buffer, sizeof(buffer), "  Домен (например, gmail.com): ");
        if (strlen(buffer) == 0) break;
        strncpy(emails[count].domain, buffer, DOMAIN_LEN - 1);
        emails[count].domain[DOMAIN_LEN - 1] = '\0';

        count++;
    }
    return count;
}

void inputAddress(Address* addr) {
    char buffer[BIG_LEN];
    printf("--- Ввод адреса (Enter - пропустить поле) ---\n");
    safeInput(buffer, sizeof(buffer), "  Страна: ");
    strncpy(addr->country, buffer, BIG_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "  Город: ");
    strncpy(addr->city, buffer, BIG_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "  Улица: ");
    strncpy(addr->street, buffer, BIG_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "  Дом: ");
    strncpy(addr->homeNumber, buffer, LOW_LEN - 1);
}

void inputJob(Job* job) {
    char buffer[COMPANY_LEN];
    printf("--- Ввод места работы (Enter - пропустить) ---\n");
    safeInput(buffer, sizeof(buffer), "  Компания: ");
    strncpy(job->company, buffer, COMPANY_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "  Должность: ");
    strncpy(job->post, buffer, POST_LEN - 1);
}

int inputSocials(Socials* socials) {
    int count = 0;
    printf("--- Ввод соцсетей (пустое название сайта для завершения) ---\n");
    while (count < SOCIALS_AMOUNT) {
        char buffer[LINK_LEN];
        safeInput(buffer, sizeof(buffer), "  Название сайта/сервиса: ");
        if (strlen(buffer) == 0) break;
        strncpy(socials[count].site, buffer, SITE_LEN - 1);
        
        safeInput(buffer, sizeof(buffer), "  Ссылка/Никнейм: ");
        if (strlen(buffer) == 0) break;
        strncpy(socials[count].link, buffer, LINK_LEN - 1);
        
        count++;
    }
    return count;
}

Person* createPersonFromInput() {
    Name name; inputName(&name);
    Phone phones[PHONES_AMOUNT]; int phoneCount = inputPhones(phones);
    Email emails[EMAILS_AMOUNT]; int emailCount = inputEmails(emails);
    
    Address addr; memset(&addr, 0, sizeof(Address)); inputAddress(&addr);
    Job job; memset(&job, 0, sizeof(Job)); inputJob(&job);
    
    Socials socials[SOCIALS_AMOUNT]; int socialCount = inputSocials(socials);

    return makePerson(&name, phoneCount, phones, emailCount, emails, &addr, &job, socialCount, socials);
}

// =========================================================
// ФУНКЦИИ ВВОДА ДЛЯ РЕДАКТИРОВАНИЯ (ОДИН ЭЛЕМЕНТ)
// =========================================================

void inputSinglePhone(Phone* ph) {
    safeInputUint16("  Код региона: ", &ph->regionCode);
    safeInputUint32("  Номер: ", &ph->number);
}

void inputSingleEmail(Email* e) {
    char buffer[64];
    safeInput(buffer, sizeof(buffer), "  Локальная часть: ");
    strncpy(e->name, buffer, NAME_LEN - 1);
    safeInput(buffer, sizeof(buffer), "  Домен: ");
    strncpy(e->domain, buffer, DOMAIN_LEN - 1);
}

void inputSingleSocial(Socials* s) {
    char buffer[128];
    safeInput(buffer, sizeof(buffer), "  Название сайта: ");
    strncpy(s->site, buffer, SITE_LEN - 1);
    safeInput(buffer, sizeof(buffer), "  Ссылка/Никнейм: ");
    strncpy(s->link, buffer, LINK_LEN - 1);
}

// =========================================================
// ФУНКЦИИ ВВОДА ДЛЯ ПОИСКА (МЯГКИЕ)
// =========================================================

void inputNameSearch(Name* n) {
    char buffer[64];
    printf("\n--- Поиск по ФИО (допускается частичный ввод) ---\n");
    safeInput(buffer, sizeof(buffer), "Имя (Enter - пропустить): ");
    strncpy(n->firstname, buffer, FN_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "Фамилия (Enter - пропустить): ");
    strncpy(n->lastname, buffer, LN_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "Отчество (Enter - пропустить): ");
    strncpy(n->surname, buffer, SN_LEN - 1);
}

void inputPhoneSearch(Phone* p) {
    printf("\n--- Поиск по телефону (требуется точное совпадение) ---\n");
    safeInputUint16("Код региона: ", &p->regionCode);
    safeInputUint32("Номер: ", &p->number);
}

void inputEmailSearch(Email* e) {
    char buffer[64];
    printf("\n--- Поиск по Email (требуется точное совпадение) ---\n");
    safeInput(buffer, sizeof(buffer), "Локальная часть (до @): ");
    strncpy(e->name, buffer, NAME_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "Домен (например, gmail.com): ");
    strncpy(e->domain, buffer, DOMAIN_LEN - 1);
}

void inputSocialSearch(Socials* s) {
    char buffer[128];
    printf("\n--- Поиск по соцсетям (допускается частичный ввод) ---\n");
    safeInput(buffer, sizeof(buffer), "Название сайта (Enter - пропустить): ");
    strncpy(s->site, buffer, SITE_LEN - 1);
    
    safeInput(buffer, sizeof(buffer), "Ссылка/Никнейм (Enter - пропустить): ");
    strncpy(s->link, buffer, LINK_LEN - 1);
}


// =========================================================
// МЕНЮ УПРАВЛЕНИЯ
// =========================================================

void addContactMenu(Contacts* book) {
    printf("\n--- Создание нового контакта ---\n");
    Person* p = createPersonFromInput();
    if (!p) return;
    
    int8_t res = addContact(book, p);
    erasePerson(p); 
    if (res >= 0) printf("[+] Контакт успешно внесен в базу.\n");
    else printf("[!] Ошибка базы данных (Код: %d).\n", res);
}

void deleteContactById(Contacts* book) {
    uint32_t id;
    if (!safeInputUint32("\nВведите ID контакта для удаления: ", &id)) return;
    
    int8_t res = removeContact(book, id);
    if (res >= 0) printf("[-] Контакт с ID %u успешно удален.\n", id);
    else printf("[!] Ошибка: Контакт с таким ID не найден (Код: %d).\n", res);
}

// =========================================================
// НОВОЕ МЕНЮ РЕДАКТИРОВАНИЯ КОНТАКТА
// =========================================================
void editContactMenu(Contacts* book) {
    uint32_t id;
    if (!safeInputUint32("\nВведите ID контакта для редактирования: ", &id)) return;
    
    Person* p = getContact(book, id);
    if (!p) {
        printf("[!] Контакт с таким ID не найден.\n");
        return;
    }

    printf("\n--- Текущие данные контакта ---\n");
    prcont(book, id);
    
    int choice = 0;
    printf("\n--- Что вы хотите изменить? ---\n");
    printf("1. ФИО (полностью)\n");
    printf("2. Конкретный телефон (по индексу)\n");
    printf("3. Конкретный email (по индексу)\n");
    printf("4. Адрес\n");
    printf("5. Конкретную соцсеть (по индексу)\n");
    printf("6. Место работы\n");
    printf("7. Заменить весь контакт (полностью)\n");
    printf("0. Отмена\n");
    if (!safeInputInt("Выберите действие: ", &choice) || choice == 0) return;

    int8_t result = 0;
    char prompt[64];
    switch (choice) {
        case 1: {
            Name newName;
            inputName(&newName);
            result = changeContact(book, id, 1, NAME, newName);
            break;
        }
        case 2: {
            if (p->phoneCount == 0) {
                printf("[!] У контакта нет телефонов.\n");
                return;
            }
            int idx;
            snprintf(prompt, sizeof(prompt), "Введите индекс телефона (0..%d): ", p->phoneCount - 1);
            if (!safeInputInt(prompt, &idx) || idx < 0 || idx >= p->phoneCount) {
                printf("[!] Неверный индекс.\n");
                return;
            }
            Phone newPhone;
            inputSinglePhone(&newPhone);
            result = changeContact(book, id, 2, PHONE, idx, newPhone);
            break;
        }
        case 3: {
            if (p->emailsCount == 0) {
                printf("[!] У контакта нет email-ов.\n");
                return;
            }
            int idx;
            snprintf(prompt, sizeof(prompt), "Введите индекс email (0..%d): ", p->emailsCount - 1);
            if (!safeInputInt(prompt, &idx) || idx < 0 || idx >= p->emailsCount) {
                printf("[!] Неверный индекс.\n");
                return;
            }
            Email newEmail;
            inputSingleEmail(&newEmail);
            result = changeContact(book, id, 2, EMAIL, idx, newEmail);
            break;
        }
        case 4: {
            Address newAddr;
            memset(&newAddr, 0, sizeof(Address));
            inputAddress(&newAddr);
            result = changeContact(book, id, 1, ADDRESS, newAddr);
            break;
        }
        case 5: {
            if (p->socialsCount == 0) {
                printf("[!] У контакта нет соцсетей.\n");
                return;
            }
            int idx;
            snprintf(prompt, sizeof(prompt), "Введите индекс соцсети (0..%d): ", p->socialsCount - 1);
            if (!safeInputInt(prompt, &idx) || idx < 0 || idx >= p->socialsCount) {
                printf("[!] Неверный индекс.\n");
                return;
            }
            Socials newSocial;
            inputSingleSocial(&newSocial);
            result = changeContact(book, id, 2, SOCIALS, idx, newSocial);
            break;
        }
        case 6: {
            Job newJob;
            memset(&newJob, 0, sizeof(Job));
            inputJob(&newJob);
            result = changeContact(book, id, 1, JOB, newJob);
            break;
        }
        case 7: {
            printf("--- Введите новые данные для полной замены контакта ---\n");
            Person* newPerson = createPersonFromInput();
            if (!newPerson) {
                printf("[!] Ошибка создания нового контакта.\n");
                return;
            }
            result = changeContact(book, id, 1, PERSON, *newPerson);
            erasePerson(newPerson);
            break;
        }
        default:
            printf("[!] Неверный выбор.\n");
            return;
    }

    if (result >= 0) {
        printf("[+] Контакт успешно обновлён.\n");
        printf("\n--- Обновлённые данные ---\n");
        prcont(book, id);
    } else {
        printf("[!] Ошибка обновления (код: %d).\n", result);
    }
}

// =========================================================
// ИСПРАВЛЕННОЕ МЕНЮ ПОИСКА
// =========================================================

void searchContactMenu(Contacts* book) {
    printf("\n--- Расширенный поиск контактов ---\n");
    printf("1. Найти по ФИО\n");
    printf("2. Найти по номеру телефона\n");
    printf("3. Найти по Email\n");
    printf("4. Найти по адресу\n");
    printf("5. Найти по профилю в соцсети\n");
    printf("6. Найти по месту работы\n");
    printf("7. Комбинированный поиск (ФИО + Место работы)\n");
    printf("0. Отмена\n");
    
    int choice;
    if (!safeInputInt("Выберите критерий поиска: ", &choice) || choice == 0) return;

    uint32_t retsz = 0;
    uint32_t* foundIds = NULL;

    switch (choice) {
        case 1: {
            Name searchName; memset(&searchName, 0, sizeof(Name));
            inputNameSearch(&searchName);
            foundIds = searchPerson(book, &retsz, 1, NAME, searchName);
            break;
        }
        case 2: {
            Phone searchPhone; memset(&searchPhone, 0, sizeof(Phone));
            inputPhoneSearch(&searchPhone);
            foundIds = searchPerson(book, &retsz, 1, PHONE, 1, searchPhone);
            break;
        }
        case 3: {
            Email searchEmail; memset(&searchEmail, 0, sizeof(Email));
            inputEmailSearch(&searchEmail);
            foundIds = searchPerson(book, &retsz, 1, EMAIL, 1, searchEmail);
            break;
        }
        case 4: {
            Address searchAddr; memset(&searchAddr, 0, sizeof(Address));
            inputAddress(&searchAddr); // Reuse default input, skips are allowed
            foundIds = searchPerson(book, &retsz, 1, ADDRESS, searchAddr);
            break;
        }
        case 5: {
            Socials searchSocial; memset(&searchSocial, 0, sizeof(Socials));
            inputSocialSearch(&searchSocial);
            foundIds = searchPerson(book, &retsz, 1, SOCIALS, 1, searchSocial);
            break;
        }
        case 6: {
            Job searchJob; memset(&searchJob, 0, sizeof(Job));
            inputJob(&searchJob);
            foundIds = searchPerson(book, &retsz, 1, JOB, searchJob);
            break;
        }
        case 7: {
            Name searchName; memset(&searchName, 0, sizeof(Name));
            Job searchJob; memset(&searchJob, 0, sizeof(Job));
            
            inputNameSearch(&searchName);
            inputJob(&searchJob);
            foundIds = searchPerson(book, &retsz, 2, NAME, searchName, JOB, searchJob);
            break;
        }
        default:
            printf("[!] Неверный выбор.\n");
            return;
    }

    // Вывод результатов
    if (foundIds && retsz > 0) {
        printf("\n[*] Найдено совпадений: %u\n", retsz);
        for (uint32_t i = 0; i < retsz; i++) {
            printf("---========---\n");
            prcont(book, foundIds[i]);
        }
        free(foundIds);
    } else {
        printf("\n[-] Контакты по заданным критериям не найдены.\n");
        if (foundIds) free(foundIds);
    }
}


// =========================================================
// ГЛАВНЫЙ ЦИКЛ
// =========================================================

// =========================================================
// ГЛАВНОЕ МЕНЮ
// =========================================================

void printMenu() {
    printf("\n=================================\n");
    printf("      ТЕЛЕФОННАЯ КНИГА           \n");
    printf("=================================\n");
    printf("1. Добавить контакт\n");
    printf("2. Удалить контакт по ID\n");
    printf("3. Вывести краткий список контактов\n");
    printf("4. Вывести подробный список контактов\n");
    printf("5. Найти и вывести контакт по ID (прямой поиск)\n");
    printf("6. Расширенный поиск по полям (ФИО, Телефон и др.)\n");
    printf("7. Редактировать контакт\n");
    printf("0. Выход\n");
    printf("=================================\n");
}

int main() {
    Contacts* book = makeContacts();
    if (!book) {
        printf("[!] Критическая ошибка: Не удалось инициализировать телефонную книгу.\n");
        return 1;
    }

    int choice = -1;
    while (choice != 0) {
        printMenu();
        if (!safeInputInt("Выберите пункт меню: ", &choice)) continue;

        switch (choice) {
            case 1: addContactMenu(book); break;
            case 2: deleteContactById(book); break;
            case 3: prshcn(book); break;
            case 4: prlncn(book); break;
            case 5: {
                uint32_t id;
                if (safeInputUint32("\nВведите ID для прямого поиска: ", &id)) prcont(book, id);
                break;
            }
            case 6: searchContactMenu(book); break;
            case 7: editContactMenu(book); break;
            case 0: printf("\nЗавершение программы.\n"); break;
            default: printf("[!] Неверный пункт меню.\n"); break;
        }
    }

    eraseContacts(book);
    return 0;
}