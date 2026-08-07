#include "lib/consumer.h"
#include "lib/generator.h"
#include "lib/err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHMEM_NAME "/tmp/shmem.m"
#define PROJ_ID 13002
#define SHMEM_NAME_SIZE 256
char mem_fname[SHMEM_NAME_SIZE] = "/tmp/shmem.m";

int main(int argc, char* argv[]){
    if(argc != 2 && argc != 3){
        fprintf(stdout, "gncs <-c | -g> [MEM_FPATH]\n");
        return ARGERR;
    }
        
    err r;

    if(argc == 3 && argv[2][0] != '\0')
        strncpy(mem_fname, argv[2], SHMEM_NAME_SIZE);
    
    if(strcmp(argv[1],"-c") == 0 || strcmp(argv[1], "--consumer") == 0)
        r = consumer_main(PROJ_ID, mem_fname);
    else if(strcmp(argv[1],"-g") == 0 || strcmp(argv[1], "--generator") == 0)
        r = generator_main(PROJ_ID, mem_fname);
    else{
        fprintf(stderr, "Such flag is not allowed. Use: -g or -c\n");
        return ARGERR;
    }

    switch (r)
    {
    case ARGERR:        fprintf(stderr, "ARGERR\n");             break;
    case MEM_ALLOC_ERR: fprintf(stderr, "MEM_ALLOC_ERR\n");      break;
    case OPEN_ERR:      fprintf(stderr, "OPEN_ERR\n");           break;
    case MAP_ERR:       fprintf(stderr, "MAP_ERR\n");            break;
    case UNMAP_ERR:     fprintf(stderr, "UNMAP_ERR\n");          break;
    case DESTROY_ERR:   fprintf(stderr, "DESTROY_ERR\n");        break;
    case SEM_CR_ERR:    fprintf(stderr, "SEM_CR_ERR\n");         break;
    case SEM_GT_ERR:    fprintf(stderr, "SEM_GT_ERR");           break;
    case FEXISTS:       fprintf(stderr, "FEXISTS\n");            break;
    default:            fprintf(stdout, "NO ERR. CLOSING...\n"); break;    
    }

    return NOERR;    
}