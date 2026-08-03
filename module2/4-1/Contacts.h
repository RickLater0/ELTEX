#ifndef CONTACTS_H
#define CONTACTS_H
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

#include "lib/Person.h"

#define PERSON 1
#define NAME 2
#define PHONE 3
#define EMAIL 4
#define ADDRESS 5
#define SOCIALS 6
#define JOB 7

typedef struct Contacts Contacts;

Contacts* makeContacts(void);
void eraseContacts(Contacts*);

int8_t    addContact     (Contacts* contacts, const Person* const person);
int8_t    eraseContact   (Contacts* contacts, Person* person);
int8_t    removeContact  (Contacts* contacts, const uint32_t id);
Person*   getContact     (const Contacts* const contacts, const uint32_t id);
int8_t    changeContact  (Contacts* contacts, const uint32_t id, const uint8_t sz, ...);
int8_t    addPhoneTo     (Contacts* contacts, const uint32_t id, const Phone phone    );
int8_t    addEmailTo     (Contacts* contacts, const uint32_t id, const Email email    );
int8_t    addSocialTo    (Contacts* contacts, const uint32_t id, const Socials socials);
int8_t    rmPhoneFrom    (Contacts* contacts, const uint32_t id, const uint8_t ind);
int8_t    rmEmailFrom    (Contacts* contacts, const uint32_t id, const uint8_t ind);
int8_t    rmSocialFrom   (Contacts* contacts, const uint32_t id, const uint8_t ind);

uint32_t getContatsSize(const Contacts* const);

void prshcn(const Contacts* const contacts);
void prlncn(const Contacts* const contacts);
void prcont(const Contacts* const contacts, const uint32_t id);


#endif