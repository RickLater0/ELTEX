/*
Написать программу для работы со списком контактов
(телефонная книга). Хранить сведения о 

Ф.И.О. человека, 
его месте работы и должности, 
номерах телефона, 
адресах электронной почты, 
ссылки на страницы в соцсетях и 
профили в мессенджерах. 

Обязательными для заполнения являются 
фамилия и имя, 
остальные поля заполняются при необходимости. 
Для хранения данных использовать массивы.
Программа должна предоставлять возможность добавления, редактирования и удаления контакта.
*/
#include "Contacts.h"
#include "lib/Storage.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Contacts
{
    Storage* storage;
} Contacts;

Contacts *makeContacts(void)
{
    Contacts* ret = (Contacts*) malloc(sizeof(Contacts));
    if(!ret)
        return NULL;

    ret->storage = mkstor(sizeof(Person));
    if(!ret->storage){
        free(ret);
        return NULL;
    }
        
    return ret;
}

int8_t percomp(const void* const v1, const void* const v2){
    return pscomp((Person*) v1, (Person*) v2);
}

int8_t pereq(const void* const v1, const void* const v2){
    return pEq((Person*) v1, (Person*) v2);
}

void eraseContacts(Contacts* contacts){
    if(contacts){
        if(contacts->storage)
            remstor(contacts->storage);
        free(contacts);
    }
}

uint32_t getContatsSize(const Contacts* const c){
    return c ? stsize(c->storage) : 0;
}

int8_t addContact(Contacts *contacts, const Person* const person)
{
    if(!contacts || !person)
        return ST_ARG_ERR;

    return stil(contacts->storage, (void*)person);
}

int8_t eraseContact(Contacts *contacts, Person *person)
{
    if(!contacts || !person)
        return ST_ARG_ERR;
    
    return ster(contacts->storage, pereq, (void*) person);
}

int8_t removeContact(Contacts *contacts, const uint32_t id)
{
    if(!contacts)
        return ST_ARG_ERR;

    return strm(contacts->storage, id);
}

Person *getContact(const Contacts* const contacts, const uint32_t id)
{
    if(!contacts)
        return NULL;

    return (Person*)stgt(contacts->storage, id)->var;
}

int8_t changeContact(Contacts* contacts, const uint32_t id, const uint8_t sz, ...){ 
    if(sz < 1)
        return ST_ARG_ERR;
    Person* orig = getContact(contacts, id);
    if(!orig)
        return ST_NO_SUCH_IND;
    va_list arg;
    int typ;

    va_start(arg, sz);

    for(int i = 0; i < sz; i++){
        typ = va_arg(arg, int);
        switch (typ)
        {
        case PERSON:
        {
            Person p = va_arg(arg, Person);
            *orig = p;
            break;
        }
        case NAME:
            {
                Name n = va_arg(arg, Name);
                if(n.firstname[0] != '\0'){
                    strncpy(orig->name.firstname, n.firstname, FN_LEN - 1);
                    orig->name.firstname[FN_LEN - 1] = '\0';
                }
                if(n.lastname[0] != '\0'){
                    strncpy(orig->name.lastname,  n.lastname,  LN_LEN - 1);
                    orig->name.firstname[LN_LEN - 1] = '\0';
                }
                if(n.surname){
                    strncpy(orig->name.surname,   n.surname,   SN_LEN - 1);
                    orig->name.surname[SN_LEN - 1] = '\0';
                }
                break;
            }
        case PHONE:
            {
                int ind = va_arg(arg, int);
                if(ind >= orig->phoneCount){
                    va_end(arg);
                    return ST_ARG_ERR;
                }
                Phone ph = va_arg(arg, Phone);
                orig->phone[ind] = ph;
                break;
            }
        case EMAIL:
            {
                int ind = va_arg(arg, int);
                if(ind >= orig->emailsCount){
                    va_end(arg);
                    return ST_ARG_ERR;
                }
                    
                Email e = va_arg(arg, Email);
                orig->email[ind] = e;
                break;
            }
        case ADDRESS:
            {
                Address ad = va_arg(arg, Address);
                orig->address = ad;
                break;
            }
        case SOCIALS:
            {
                int ind = va_arg(arg, int);
                if(ind >= orig->socialsCount){
                    va_end(arg);
                    return ST_ARG_ERR;
                }
                Socials so = va_arg(arg, Socials);
                orig->socials[ind] = so;
                break;
            }
        case JOB:
            {
                Job job = va_arg(arg, Job);
                orig->job = job;
                break; 
            }
        default://такого не знаем - выходим 
            va_end(arg);
            return ST_ARG_ERR;
        }
    }
    va_end(arg);
    return 1;
}

void prshcn(const Contacts* const contacts) {
    if (!contacts || stsize(contacts->storage) == 0) {
        printf("Книга пуста.\n");
        return;
    }

    printf("\t\t\tКонтакты\n");
    
    StIterator* it = stbeg(contacts->storage);
    uint32_t total = stsize(contacts->storage);
    uint32_t current = 0;
    StIterator* end = stend(contacts->storage);
    while (it && itcomp(it, end) != 0) {
        StEntry* entry = stitgt(it);
        if (entry) {
            Person* p = (Person*)entry->var;
            char* pers = ptstr(p);
            
            printf("id: %u\n%s\n", entry->id, pers);
            free(pers);

            current++;
            if (current < total) {
                printf("---========---\n");
            }
        }
        stnext(it);
    }
    rmiter(end);
    rmiter(it);
}

void prlncn(const Contacts* const contacts){
    if (!contacts || stsize(contacts->storage) == 0) {
        printf("Книга пуста.\n");
        return;
    }

    printf("\t\t\tКонтакты\n");
    
    StIterator* it = stbeg(contacts->storage);
    uint32_t total = stsize(contacts->storage);
    uint32_t current = 0;
    StIterator* end = stend(contacts->storage);
    while (it && itcomp(it, end) != 0) {
        StEntry* entry = stitgt(it);
        if (entry) {
            Person* p = (Person*)entry->var;
            char* pers = ptfstr(p);
            
            printf("id: %u\n%s\n", entry->id, pers);
            free(pers);

            current++;
            if (current < total) {
                printf("---========---\n");
            }
        }
        stnext(it);
    }
    rmiter(it);
    rmiter(end);
}

void prcont(const Contacts* const contacts, const uint32_t id){
    Person* p = getContact(contacts, id);
    char* pers;
    if(!p){
        printf("Пользователь с этим id не найден\n");
        return;
    }
    pers = ptfstr(p);
    printf("id: %d\n%s\n", id, pers);
    free(pers);
}

uint8_t checkFlag(uint8_t flags, int check){
    return flags & (1 << check); 
}

uint8_t strIsEmpty(char* str){
    return str == NULL || *str == '\0';
}

//Пример ввода
//contacts, sz, 2 NAME, name, ADDRESS, address 
uint32_t* searchPerson(const Contacts* const contacts, uint32_t* retsz, uint8_t n, ...){
    if(!contacts || !retsz || n < 1)
        return NULL;

    Person john;
    uint8_t flags = 0;
    memset(&john, 0, sizeof(Person));
    va_list arg;
    va_start(arg, n);
    for(uint8_t i = 0; i < n; i++){
        int cs = va_arg(arg, int);
        switch (cs)
        {
        case PERSON:
            john = va_arg(arg, Person);
            flags = ~0x01;//0xFE - включаем все флаги
            break;
        case NAME:
            {
                john.name = va_arg(arg, Name);
                flags |= 1 << NAME;//0x02
                break;
            }
        case PHONE:
            {
                int sz = va_arg(arg, int);
                if(sz <= 0 || sz >= PHONES_AMOUNT){
                    va_end(arg);
                    return NULL;
                }
                flags |= 1 << PHONE;
                for(uint8_t i = 0; i < sz; i++){
                    addPhone(&john, va_arg(arg, Phone));
                }
                break;
            }
        case EMAIL:
            {
                int sz = va_arg(arg, int);
                if(sz <= 0 || sz >= EMAILS_AMOUNT){
                    va_end(arg);
                    return NULL;
                }
                flags |= 1 << EMAIL;
                for(uint8_t i = 0; i < sz; i++){
                    addEmail(&john, va_arg(arg, Email));
                }
                break;
            }
        case ADDRESS:
            {
                flags |= 1 << ADDRESS;
                john.address = va_arg(arg, Address);
                break;
            }
        case SOCIALS:
            {
                int sz = va_arg(arg, int);
                if(sz <= 0 || sz >= SOCIALS_AMOUNT){
                    va_end(arg);
                    return NULL;
                }
                flags |= 1 << SOCIALS;
                for(uint8_t i = 0; i < sz; i++){
                    addSocials(&john, va_arg(arg, Socials));
                }
                break;
            }
        case JOB:
            {
                flags |= 1 << JOB;
                john.job = va_arg(arg, Job);
                break; 
            }
        default://такого не знаем - выходим 
            va_end(arg);
            return NULL;
        }
    }
    va_end(arg);

    *retsz = 0;
    uint32_t* ids = stfa(contacts->storage, percomp, &john, retsz);

    return ids;
}

int8_t    addPhoneTo     (Contacts* contacts, const uint32_t id, const Phone phone    ){
    Person* p = getContact(contacts, id);
    return addPhone(p, phone);
}
int8_t    addEmailTo     (Contacts* contacts, const uint32_t id, const Email email    ){
    Person* p = getContact(contacts, id);
    return addEmail(p, email);
}
int8_t    addSocialTo    (Contacts* contacts, const uint32_t id, const Socials socials){
    Person* p = getContact(contacts, id);
    return addSocials(p, socials);   
}
int8_t    rmPhoneFrom    (Contacts* contacts, const uint32_t id, const uint8_t ind){
    Person* p = getContact(contacts, id);
    return remPhone(p, ind);
}
int8_t    rmEmailFrom    (Contacts* contacts, const uint32_t id, const uint8_t ind){
    Person* p = getContact(contacts, id);
    return remEmail(p, ind);
}
int8_t    rmSocialFrom   (Contacts* contacts, const uint32_t id, const uint8_t ind){
    Person* p = getContact(contacts, id);
    return remSocial(p, ind);
}