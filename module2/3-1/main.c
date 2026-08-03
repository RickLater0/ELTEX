#include "chmo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char line[3 * STRMAX];

int main(){
    char fname[STRMAX];
    char args[STRMAX];
    for(;;){
        if (!fgets(line, sizeof(line), stdin)) break;

        line[strcspn(line, "\n")] = '\0';
        *fname = '\0';
        *args  = '\0';
        int n = sscanf(line, "%32s %32s", fname, args);

        if (n == 0) continue; // пустая строка

        if (strcmp(fname, "exit") == 0) {
            break;
        }

        file f;
        switch (chmo(fname, args, &f))
        {
        case OVERLOAD:
            printf("Невозможно записать новый файл %s - достигнут лимит %d\n", fname, FMAX);
            break;
        case NOERR:
        case NOARG:

            printf("%s - ", f.name);
            uint16_t b = f.rights; 
            for (int i = 0; i < 9; i++) {
                char c = '-';
                if(b & 0x100){
                    switch (i % 3)
                    {
                    case 0:
                        c = 'r';
                        break;
                    case 1:
                        c = 'w';
                        break;
                    case 2:
                        c = 'x';
                        break;
                    default:
                        break;
                    }
                }
                printf("%c", c);
                b <<= 1;
            }
            printf(" ");
            b = f.rights; 
            for (int i = 0; i < 9; i++) {
                printf("%c", (b & 0x100) ? '1' : '0');
                b <<= 1;
            }
            
            printf(" %o\n", f.rights);
            break;
        case ARGERR:
            printf("ARGERR\n");
            break;
        }
    }
}
