#include "Person.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//-----------------ADDRESS-----------------

void initAddress(Address* addr, const char* country, const char* city, const char* street, const char* homeNumber) {
    if (!addr) return;
    memset(addr, 0, sizeof(Address)); // Очищаем мусор

    if (country) { 
        strncpy(addr->country, country, BIG_LEN - 1); 
        addr->country[BIG_LEN - 1] = '\0'; 
    }
    if (city) { 
        strncpy(addr->city, city, BIG_LEN - 1); 
        addr->city[BIG_LEN - 1] = '\0'; 
    }
    if (street) { 
        strncpy(addr->street, street, BIG_LEN - 1); 
        addr->street[BIG_LEN - 1] = '\0'; 
    }
    if (homeNumber) { 
        strncpy(addr->homeNumber, homeNumber, LOW_LEN - 1); 
        addr->homeNumber[LOW_LEN - 1] = '\0'; 
    }
}

static void changeAll(Address* addr, char* country, char* city, char* street, char* homeNumber){
    if(!addr)
        return;

    if (country && addr->country != country) {
        strncpy(addr->country, country, BIG_LEN - 1);
        addr->country[BIG_LEN - 1] = '\0';
    }

    if (city && addr->city != city) {
        strncpy(addr->city, city, BIG_LEN - 1);
        addr->city[BIG_LEN - 1] = '\0';
    }

    if (street && addr->street != street) {
        strncpy(addr->street, street, BIG_LEN - 1);
        addr->street[BIG_LEN - 1] = '\0';
    }

    if (homeNumber && addr->homeNumber != homeNumber) {
        strncpy(addr->homeNumber, homeNumber, LOW_LEN - 1);
        addr->homeNumber[LOW_LEN - 1] = '\0';
    }
}

void changeCountry(Address* addr, char* country, char* city, char* street, char* homeNumber)
{
    changeAll(addr, country, city, street, homeNumber);
}

void changeCity(Address* addr, char *city, char *street, char *homeNumber)
{    
    changeAll(addr, addr->country, city, street, homeNumber);
}

void changeStreet(Address* addr, char* street, char* homeNumber){
    changeAll(addr, addr->country, addr->city, street, homeNumber);
}

void changeHomeNumber(Address* addr, char* homeNumber){
    changeAll(addr, addr->country, addr->city, addr->street, homeNumber);
} 

void changeHomeNumberInt(Address* addr, uint16_t homeNumber){
    if(!addr)
        return;
    snprintf(addr->homeNumber, LOW_LEN, "%u", homeNumber);
}

int8_t addrEq(const Address* const a1, const Address* const a2) {
    if(a1 == a2)
        return 1;
        
    if(!a1 || !a2)
        return 0;

    if(strcmp(a1->homeNumber, a2->homeNumber) != 0)
        return 0;
    if(strcmp(a1->street, a2->street) != 0)
        return 0;
    if(strcmp(a1->city, a2->city) != 0)
        return 0;
    if(strcmp(a1->country, a2->country) != 0)
        return 0;
    return 1;
}
//strstr для каждого поля 
int8_t addrSoftEq(const Address* const a1, const Address* const a2) {
    if (!a1 || !a2) return 0;

    int has_search = 0;
    if (a2->homeNumber[0] != '\0') {
        has_search = 1;
        if (strstr(a1->homeNumber, a2->homeNumber)) return 1;
    }
    if (a2->street[0] != '\0') {
        has_search = 1;
        if (strstr(a1->street, a2->street)) return 1;
    }
    if (a2->city[0] != '\0') {
        has_search = 1;
        if (strstr(a1->city, a2->city)) return 1;
    }
    if (a2->country[0] != '\0') {
        has_search = 1;
        if (strstr(a1->country, a2->country)) return 1;
    }
    
    return has_search ? 0 : 1;
}

//-----------------EMAIL-----------------

void initEmail(Email* email, const char* name, const char* domain) {
    if (!email) return;
    memset(email, 0, sizeof(Email));

    if (name) { 
        strncpy(email->name, name, NAME_LEN - 1); 
        email->name[NAME_LEN - 1] = '\0'; 
    }
    if (domain) { 
        strncpy(email->domain, domain, DOMAIN_LEN - 1); 
        email->domain[DOMAIN_LEN - 1] = '\0'; 
    }
}

int8_t emailEq(const Email* const e1, const Email* const e2){
    if(e1 == e2)
        return 1;
        
    if(!e1 || !e2)
        return 0;

    if(strcmp(e1->domain, e2->domain) != 0)
        return 0;
    if(strcmp(e1->name, e2->name) != 0)
        return 0;

    return 1;
}

//-----------------JOB----------------

void initJob(Job* job, const char* company, const char* post) {
    if (!job) return;
    memset(job, 0, sizeof(Job));

    if (company) { 
        strncpy(job->company, company, COMPANY_LEN - 1); 
        job->company[COMPANY_LEN - 1] = '\0'; 
    }
    if (post) { 
        strncpy(job->post, post, POST_LEN - 1); 
        job->post[POST_LEN - 1] = '\0'; 
    }
}

int8_t jEq(const Job* const s1, const Job* const s2){
    if(s1 == s2)
        return 1;
    if(!s1 || !s2)
        return 0;

    if(strcmp(s1->company, s2->company) != 0)
        return 0;
    if(strcmp(s1->post, s2->post) != 0)
        return 0;
    
    return 1;
}

int8_t jseq(const Job* const s1, const Job* const s2) {
    if (!s1 || !s2) return 0;

    int has_search = 0;
    if (s2->company[0] != '\0') {
        has_search = 1;
        if (strstr(s1->company, s2->company)) return 1;
    }
    if (s2->post[0] != '\0') {
        has_search = 1;
        if (strstr(s1->post, s2->post)) return 1;
    }
    
    return has_search ? 0 : 1;
}

//-----------------NAME----------------

void initName(Name* n, const char* firstName, const char* lastName, const char* surname) {
    if (!n || !firstName || !lastName) return; // Имя и фамилия обязательны
    memset(n, 0, sizeof(Name));

    strncpy(n->firstname, firstName, FN_LEN - 1);
    n->firstname[FN_LEN - 1] = '\0';
    
    strncpy(n->lastname, lastName, LN_LEN - 1);
    n->lastname[LN_LEN - 1] = '\0';

    if (surname) { 
        strncpy(n->surname, surname, SN_LEN - 1); 
        n->surname[SN_LEN - 1] = '\0'; 
    }
}

int8_t namesEq(const Name* const n1, const Name* const n2){

    if(n1 == n2)
        return 1;
    
    if(!n1 || !n2)
        return 0;
        
    if(strcmp(n1->firstname, n2->firstname) != 0)
        return 0;
    if(strcmp(n1->lastname , n2->lastname) != 0)
        return 0;
    if(strcmp(n1->surname  , n2->surname) != 0)
        return 0;
    return 1;    

}

int8_t nmcmp(const Name* const n1, const Name* const n2){

    int8_t cmp1 = strcmp(n1->firstname, n2->firstname);
    int8_t cmp2 = strcmp(n1->lastname , n2->lastname);
    int8_t cmp3 = strcmp(n1->surname  , n2->surname);
    return cmp1 != 0 ? cmp1 : (cmp2 != 0 ? cmp2 : cmp3);
}

int8_t nseq(const Name* const n1, const Name* const n2) {
    if (!n1 || !n2) return 0;
    
    int has_search = 0;
    if (n2->firstname[0] != '\0') {
        has_search = 1;
        if (strstr(n1->firstname, n2->firstname)) return 1;
    }
    if (n2->lastname[0] != '\0') {
        has_search = 1;
        if (strstr(n1->lastname, n2->lastname)) return 1;
    }
    if (n2->surname[0] != '\0') {
        has_search = 1;
        if (strstr(n1->surname, n2->surname)) return 1;
    }
    
    // Если критерии поиска по имени вообще не передавались — пропускаем проверку (считаем совпадением)
    return has_search ? 0 : 1; 
}
int8_t naseq    (const Name* const n1, const char* const str){
    if(!n1 || !str)
        return 0;
        
    if(strstr(n1->firstname, str))
        return 1;
    if(strstr(n1->lastname , str))
        return 1;
    if(strstr(n1->surname  , str))
        return 1;

    return 0;   
}

//-----------------PHONE-----------------

void initPhone(Phone* phone, uint16_t region, uint32_t number) {
    if (!phone) return;
    memset(phone, 0, sizeof(Phone)); // Для примитивных типов достаточно memset
    
    phone->regionCode = region;
    phone->number = number;
}

int8_t phEq(const Phone* const p1, const Phone* const p2){
    if(p1 == p2)
        return 1;
        
    if(!p1 || !p2)
        return 0;
    
    if(p1->regionCode != p2->regionCode)
        return 0;
    if(p1->number != p2->number)
        return 0;

    return 1;
}

//-----------------SOCIALS-----------------

void initSocials(Socials* soc, const char* site, const char* link) {
    if (!soc) return;
    memset(soc, 0, sizeof(Socials));

    if (site) { 
        strncpy(soc->site, site, SITE_LEN - 1); 
        soc->site[SITE_LEN - 1] = '\0'; 
    }
    if (link) { 
        strncpy(soc->link, link, LINK_LEN - 1); 
        soc->link[LINK_LEN - 1] = '\0'; 
    }
}

int8_t soEq(const Socials* const s1, const Socials* const s2){
    if(s1 == s2)
        return 1;
    if(!s1 || !s2)
        return 0;

    if(strcmp(s1->link, s2->link) != 0)
        return 0;
    if(strcmp(s1->site, s2->site) != 0)
        return 0;
    
    return 1;
}

int8_t soseq(const Socials* const s1, const Socials* const s2){
    if(s1 == s2)
        return 1;
    if(!s1 || !s2)
        return 0;

    if(strstr(s1->link, s2->link))
        return 1;
    if(strstr(s1->site, s2->site))
        return 1;
    
    return 0;
}

//-----------------PERSON-----------------

Person* makePerson(
        const Name* const name, 
        const size_t phonesCount,
        const Phone* const ph,
        const size_t emailsCount,
        const Email* const email, 
        const Address* const addr, 
        const Job* const job,
        const size_t socialsCount, 
        const Socials* const soc
    )
{
    if(name == NULL)
        return NULL;

    Person* ret = malloc(sizeof(Person));
    memset(ret, 0, sizeof(Person));
    ret->name = *name;
    
    if(ph && phonesCount > 0){
        size_t toCopy = (phonesCount < PHONES_AMOUNT) ? phonesCount : PHONES_AMOUNT;
        memcpy(ret->phone, ph, toCopy * sizeof(Phone));
        ret->phoneCount = (uint8_t)toCopy;
    }
    if(email && emailsCount > 0){
        size_t toCopy = (emailsCount < EMAILS_AMOUNT) ? emailsCount : EMAILS_AMOUNT;
        memcpy(ret->email, email, toCopy * sizeof(Email));
        ret->emailsCount = (uint8_t)toCopy;
    }
    if(soc && socialsCount > 0){
        size_t toCopy = (socialsCount < SOCIALS_AMOUNT) ? socialsCount : SOCIALS_AMOUNT;
        memcpy(ret->socials, soc, toCopy * sizeof(Socials));
        ret->socialsCount = (uint8_t)toCopy;
    }
        if (addr) {
        ret->address = *addr;
    }
    if (job) {
        ret->job = *job;
    }
    return ret;
}

void initPerson(
    Person* p,
    const Name* const name, 
    const size_t phonesCount,
    const Phone* const ph,
    const size_t emailsCount,
    const Email* const email, 
    const Address* const addr, 
    const Job* const job,
    const size_t socialsCount, 
    const Socials* const soc
) {
    if (!p || !name) return;

    memset(p, 0, sizeof(Person));
    
    // Копируем обязательное поле
    p->name = *name;
    
    // Копируем массивы через memcpy, защищаясь от переполнения
    if (ph && phonesCount > 0) {
        size_t toCopy = (phonesCount < PHONES_AMOUNT) ? phonesCount : PHONES_AMOUNT;
        memcpy(p->phone, ph, toCopy * sizeof(Phone));
        p->phoneCount = (uint8_t)toCopy;
    }
    
    if (email && emailsCount > 0) {
        size_t toCopy = (emailsCount < EMAILS_AMOUNT) ? emailsCount : EMAILS_AMOUNT;
        memcpy(p->email, email, toCopy * sizeof(Email));
        p->emailsCount = (uint8_t)toCopy;
    }
    
    if (soc && socialsCount > 0) {
        size_t toCopy = (socialsCount < SOCIALS_AMOUNT) ? socialsCount : SOCIALS_AMOUNT;
        memcpy(p->socials, soc, toCopy * sizeof(Socials));
        p->socialsCount = (uint8_t)toCopy;
    }
    
    // Копируем вложенные структуры
    if (addr) p->address = *addr;
    if (job)  p->job = *job;
}

 
void erasePerson(Person* p){
    if(p)
        free(p);
}
int8_t addPhone(Person* person, const Phone phone){
    if(!person) 
        return ERRARG;
    if(person->phoneCount >= PHONES_AMOUNT)
        return OUTBOUND;
    person->phone[person->phoneCount] = phone;
    person->phoneCount++;
    return NOERR;
}

int8_t addSocials(Person* person, const Socials socials){
    if(!person) 
        return ERRARG;
    if(person->socialsCount >= SOCIALS_AMOUNT)
        return OUTBOUND;
    person->socials[person->socialsCount] = socials;
    person->socialsCount++;
    return NOERR;
}

int8_t addEmail(Person* person, const Email email){
    if(!person) 
        return ERRARG;
    if(person->emailsCount >= EMAILS_AMOUNT)
        return OUTBOUND;
    person->email[person->emailsCount] = email;
    person->emailsCount++;
    return NOERR;
}

int8_t remPhone(Person* person, const uint8_t i){
    if(i >= person->phoneCount)
        return OUTBOUND;
    uint8_t toMove = person->phoneCount - i - 1;
    memmove(&person->phone[i], &person->phone[i+1], sizeof(Phone) * toMove);
    person->phoneCount--;
    return NOERR;
}

int8_t remSocial(Person* person, const uint8_t i){
    if(i >= person->socialsCount)
        return OUTBOUND;
    uint8_t toMove = person->socialsCount - i - 1;
    memmove(&person->socials[i], &person->socials[i+1], sizeof(Socials) * toMove);
    person->socialsCount--;
    return NOERR;
}

int8_t remEmail(Person* person, const uint8_t i){
    if(i >= person->emailsCount)
        return OUTBOUND;
    uint8_t toMove = person->emailsCount - i - 1;
    memmove(&person->email[i], &person->email[i+1], sizeof(Email) * toMove);
    person->emailsCount--;
    return NOERR;
}

int8_t remLastPhone(Person* person){
    if(!person)
        return ERRARG;
    if(person->phoneCount == 0)
        return OUTBOUND;
    
    memset(&person->phone[--person->phoneCount], 0, sizeof(Phone));
    return NOERR;
}

int8_t remLastSocial(Person* person){
    if(!person)
        return ERRARG;
    if(person->socialsCount == 0)
        return OUTBOUND;
    
    memset(&person->socials[--person->socialsCount], 0, sizeof(Socials));
    return NOERR;
}

int8_t remLastEmail(Person* person){
    if(!person)
        return ERRARG;
    if(person->emailsCount == 0)
        return OUTBOUND;
    
    memset(&person->email[--person->emailsCount], 0, sizeof(Email));
    return NOERR;
}

int8_t pEq(const Person* const p1, const Person* const p2){
    if(p1 == p2)
        return 1;
        
    if(!p1 || !p2)
        return 0;

    if(!namesEq(&p1->name, &p2->name))
        return 0;
    if(!addrEq(&p1->address, &p2->address))
        return 0;

    if (p1->phoneCount != p2->phoneCount) 
        return 0;
    for (int i = 0; i < p1->phoneCount; i++) {
        if (!phEq(&p1->phone[i], &p2->phone[i])) return 0;
    }

    if (p1->emailsCount != p2->emailsCount) return 0;
    for (int i = 0; i < p1->emailsCount; i++) {
        if (!emailEq(&p1->email[i], &p2->email[i])) return 0;
    }

    if (!jEq(&p1->job, &p2->job)) 
        return 0;
    if (p1->socialsCount != p2->socialsCount) return 0;
    for (int i = 0; i < p1->socialsCount; i++) {
        if (!soEq(&p1->socials[i], &p2->socials[i])) return 0;
    }
        
    return 1;
}

int8_t pscomp(const Person* const p, const Person* const t){
    if(p == t)
        return 0;
    if(!p || !t)
        return -2;

    // nseq возвращает 0, если совпало или если текстовые поля пустые. 
    // Если вернула не 0 — значит, есть заданное поле, которое НЕ совпало.
    if (nseq(&p->name, &t->name) != 0) 
        return 1; 
        
    if (addrSoftEq(&p->address, &t->address) == 0) 
        return 1;

    if (!jseq(&p->job, &t->job)) 
        return 1;

    // Поиск по телефонам: если в шаблоне что-то ищут
    if (t->phoneCount > 0) {
        int8_t found = 0;
        for (uint8_t i = 0; i < t->phoneCount; i++) {
            for (uint8_t j = 0; j < p->phoneCount; j++) {
                // Если совпали номера (без учета regionCode, либо допишите проверку региона при необходимости)
                if (p->phone[j].number == t->phone[i].number) {
                    found = 1;
                    break;
                }
            }
        }
        if (!found) return 1; // Заданный телефон не найден в контакте
    }

    // Поиск по email
    if (t->emailsCount > 0) {
        int8_t found = 0;
        for (uint8_t i = 0; i < t->emailsCount; i++) {
            for (uint8_t j = 0; j < p->emailsCount; j++) {
                if (emailEq(&p->email[j], &t->email[i])) {
                    found = 1;
                    break;
                }
            }
        }
        if (!found) return 1;
    }

    // Поиск по соцсетям
    if (t->socialsCount > 0) {
        int8_t found = 0;
        for (uint8_t i = 0; i < t->socialsCount; i++) {
            for (uint8_t j = 0; j < p->socialsCount; j++) {
                if (soseq(&p->socials[j], &t->socials[i])) {
                    found = 1;
                    break;
                }
            }
        }
        if (!found) return 1;
    }

    return 0; // Полное совпадение по всем заполненным критериям!
}

char* ptstr(const Person* const person){
    if(!person)
        return NULL;

    size_t total_size = 0;

    total_size += snprintf(NULL, 0, "ФИО: %s %s %s\n", 
                           person->name.firstname, person->name.lastname, person->name.surname);

    total_size += snprintf(NULL, 0, "Телефоны: \n");
    for (int i = 0; i < person->phoneCount; i++) {
        total_size += snprintf(NULL, 0, "\t%d: +(%d) %d\n", i + 1, 
                               person->phone[i].regionCode, person->phone[i].number);
    }

    total_size += snprintf(NULL, 0, "Emails: \n");
    for (int i = 0; i < person->emailsCount; i++) {
        total_size += snprintf(NULL, 0, "\t%d: %s@%s\n", i + 1, 
                               person->email[i].name, person->email[i].domain);
    }

    char* str = malloc(total_size + 1);
    if (!str)
        return NULL;

    size_t offset = 0;
    size_t remaining = total_size + 1;
    int printed = 0;

    printed = snprintf(str + offset, remaining, "ФИО: %s %s %s\n", 
                       person->name.firstname, person->name.lastname, person->name.surname);
    offset += printed;
    remaining -= printed;

    printed = snprintf(str + offset, remaining, "Телефоны: \n");
    offset += printed;
    remaining -= printed;
    for (int i = 0; i < person->phoneCount; i++) {
        printed = snprintf(str + offset, remaining, "\t%d: +(%d) %d\n", i + 1, 
                           person->phone[i].regionCode, person->phone[i].number);
        offset += printed;
        remaining -= printed;
    }

    printed = snprintf(str + offset, remaining, "Emails: \n");
    offset += printed;
    remaining -= printed;
    for (int i = 0; i < person->emailsCount; i++) {
        printed = snprintf(str + offset, remaining, "\t%d: %s@%s\n", i + 1, 
                           person->email[i].name, person->email[i].domain);
        offset += printed;
        remaining -= printed;
    }

    return str;
}

char* ptfstr(const Person* const person){
    if(!person)
        return NULL;

    size_t total_size = 0;

    total_size += snprintf(NULL, 0, "ФИО: %s %s %s\n", 
                           person->name.firstname, person->name.lastname, person->name.surname);

    total_size += snprintf(NULL, 0, "Телефоны: \n");
    for (int i = 0; i < person->phoneCount; i++) {
        total_size += snprintf(NULL, 0, "\t%d: +(%d) %d\n", i + 1, 
                               person->phone[i].regionCode, person->phone[i].number);
    }

    total_size += snprintf(NULL, 0, "Emails: \n");
    for (int i = 0; i < person->emailsCount; i++) {
        total_size += snprintf(NULL, 0, "\t%d: %s@%s\n", i + 1, 
                               person->email[i].name, person->email[i].domain);
    }

    total_size += snprintf(NULL, 0, "Соцсети: \n");
    for (int i = 0; i < person->socialsCount; i++) {
        total_size += snprintf(NULL, 0, "\t%d: %s - %s\n", i + 1, 
                               person->socials[i].site, person->socials[i].link);
    }

    total_size += snprintf(NULL, 0, "Работа: %s - %s\n", 
                           person->job.company, person->job.post);

    total_size += snprintf(NULL, 0, "Адрес: %s, г. %s, ул. %s, д. %s\n",
                           person->address.country, person->address.city,
                           person->address.street, person->address.homeNumber);

    char* str = malloc(total_size + 1);
    if (!str)
        return NULL;

    size_t offset = 0;
    size_t remaining = total_size + 1;
    int printed = 0;

    printed = snprintf(str + offset, remaining, "ФИО: %s %s %s\n", 
                       person->name.firstname, person->name.lastname, person->name.surname);
    offset += printed;
    remaining -= printed;

    printed = snprintf(str + offset, remaining, "Телефоны: \n");
    offset += printed;
    remaining -= printed;
    for (int i = 0; i < person->phoneCount; i++) {
        printed = snprintf(str + offset, remaining, "\t%d: +%d %d\n", i + 1, 
                           person->phone[i].regionCode, person->phone[i].number);
        offset += printed;
        remaining -= printed;
    }

    printed = snprintf(str + offset, remaining, "Emails: \n");
    offset += printed;
    remaining -= printed;
    for (int i = 0; i < person->emailsCount; i++) {
        printed = snprintf(str + offset, remaining, "\t%d: %s@%s\n", i + 1, 
                           person->email[i].name, person->email[i].domain);
        offset += printed;
        remaining -= printed;
    }

    printed = snprintf(str + offset, remaining, "Соцсети: \n");
    offset += printed;
    remaining -= printed;
    for (int i = 0; i < person->socialsCount; i++) {
        printed = snprintf(str + offset, remaining, "\t%d: %s - %s\n", i + 1, 
                           person->socials[i].site, person->socials[i].link);
        offset += printed;
        remaining -= printed;
    }

    printed = snprintf(str + offset, remaining, "Работа: %s - %s\n", 
                       person->job.company, person->job.post);
    offset += printed;
    remaining -= printed;

    snprintf(str + offset, remaining, "Адрес: %s, г. %s, ул. %s, д. %s\n",
             person->address.country, person->address.city,
             person->address.street, person->address.homeNumber);

    return str;
}