#ifndef CHMO_H
#define CHMO_H

#include <string.h>
#include <stdint.h>

#define FMAX 32
#define STRMAX 64
#define NOARG 1
#define NOERR 0
#define ARGERR -1
#define OVERLOAD -2

typedef struct file
{
    char name[STRMAX];
    uint16_t rights;
} file;

int8_t chmo(char* fname, char* args, file* farg);

#endif