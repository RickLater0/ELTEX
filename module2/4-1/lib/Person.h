#ifndef PERSON_H
#define PERSON_H

#include <stdint.h>
#include <stddef.h>

enum MSG{
    ERRARG = -1,
    NOERR = 0,
    ERRIND = -2,
    OUTBOUND = -3
};

//-----------------ADDRESS-----------------
enum ADDR_CONSTANTS{
    BIG_LEN = 32,
    LOW_LEN = 16
};

typedef struct Address
{
    char country    [BIG_LEN];
    char city       [BIG_LEN];
    char street     [BIG_LEN];
    char homeNumber [LOW_LEN];
} Address;

Address* makeAddress(char* country, char* city,  char* street,  char* homeNumber);
void eraseAddress(Address* addr);

void changeCountry      (Address* addr,  char* country,  char* city,  char* street,  char* homeNumber);
void changeCity         (Address* addr,  char* city,  char* street,  char* homeNumber);
void changeStreet       (Address* addr,  char* street,  char* homeNumber);
void changeHomeNumber   (Address* addr,  char* homeNumber);
void changeHomeNumberInt   (Address* addr,  uint16_t homeNumber);

int8_t addrEq(const Address* const, const Address* const);

//-----------------EMAIL-----------------

enum EMAIL_CONSTANTS{
    NAME_LEN = 32,
    DOMAIN_LEN = 32
};

typedef struct Email
{
    char name   [NAME_LEN];
    char domain [DOMAIN_LEN];
} Email;

Email* makeEmail(char* name, char* domain);
void eraseEmail(Email* email);

int8_t emailEq(const Email* const, const Email* const);
int8_t addrSoftEq(const Address* const a1, const Address* const a2);

//-----------------JOB-----------------

enum JOB_CONSTANTS{
    COMPANY_LEN = 32,
    POST_LEN = 32
};

typedef struct Job
{
    char company[COMPANY_LEN];
    char post[POST_LEN];
} Job;

int8_t jEq(const Job* const s1, const Job* const s2);
int8_t jseq(const Job* const s1, const Job* const s2);

//-----------------NAME-----------------

enum NAME_CONSTANTS{
    FN_LEN = 16,
    LN_LEN = 16,
    SN_LEN = 16
};

typedef struct Name
{
    char firstname  [FN_LEN];   //имя
    char lastname   [LN_LEN];   //фамилия
    char surname    [SN_LEN];   //отчество
} Name;

Name* makeName(const char* const firstName, const char* const lastName, const char* const surname);
void eraseName(Name* name);

int8_t namesEq  (const Name* const, const Name* const);
int8_t nseq     (const Name* const, const Name* const);
int8_t naseq    (const Name* const, const char* const);

int8_t nmcmp(const Name* const n1, const Name* const n2);
//-----------------PHONE-----------------

typedef struct Phone
{
    uint16_t regionCode;
    uint32_t number;
} Phone;

Phone*  makePhone(uint16_t region, uint32_t number);
void    erasePhone(Phone* phone);

int8_t phEq(const Phone* const, const Phone* const);

//-----------------SOCIALS-----------------

enum SOCIALS_CONSTANTS{
    SITE_LEN = 16,
    LINK_LEN = 128
};

typedef struct Socials
{
    char site[SITE_LEN];
    char link[LINK_LEN];
} Socials;

int8_t soEq(const Socials* const s1, const Socials* const s2);
int8_t soseq(const Socials* const s1, const Socials* const s2);

//-----------------PERSON-----------------

enum PERSON_CONSTANTS{
    PHONES_AMOUNT = 4,
    EMAILS_AMOUNT = 8,
    SOCIALS_AMOUNT = 12
};

typedef struct Person{
    Name    name;
    uint8_t phoneCount;
    uint8_t emailsCount;
    Phone   phone[PHONES_AMOUNT];
    Email   email[EMAILS_AMOUNT];
    Address address;
    Job job;
    uint8_t socialsCount;
    Socials socials[SOCIALS_AMOUNT];
} Person;

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
);
void erasePerson(Person*);

int8_t pEq   (const Person* const, const Person* const);
int8_t pscomp(const Person* const, const Person* const);

char* ptstr (const Person* const);
char* ptfstr(const Person* const);

int8_t addPhone  (Person*, const Phone);
int8_t addSocials(Person*, const Socials);
int8_t addEmail  (Person*, const Email);
int8_t remPhone (Person* person, const uint8_t i);
int8_t remSocial(Person* person, const uint8_t i);
int8_t remEmail (Person* person, const uint8_t i);
int8_t remLastPhone (Person* person);
int8_t remLastSocial(Person* person);
int8_t remLastEmail (Person* person);

void initAddress(Address* addr, const char* country, const char* city, const char* street, const char* homeNumber);
void initEmail  (Email* email, const char* name, const char* domain);
void initJob    (Job* job, const char* company, const char* post);
void initName   (Name* n, const char* firstName, const char* lastName, const char* surname);
void initPhone  (Phone* phone, uint16_t region, uint32_t number);
void initSocials(Socials* soc, const char* site, const char* link);
void initPerson (
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
);
#endif