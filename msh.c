
 //msh.c - a minimal Unix shell
 //Task 1: prompt, read a line of any length, exit cleanly on Ctrl-D
 


//comes BEFORE any #include. 
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  

int main(void)
{
    char   *line = NULL;  // buffer owned by getline(); NULL = "allocate for me" 
    size_t  cap  = 0;     //current size of that buffer; getline updates it  
    ssize_t nread;        // chars read, or -1 on EOF/error (signed on purpose) 

    while (1) {
        // 1. Show the prompt. stdout is line-buffered on a terminal, so
         //    text without '\n' might not appear. fflush forces it out. 
        printf("msh> ");
        fflush(stdout);

        //2. Read one whole line. getline grows 'line' as needed,so the line can be any length. 
        nread = getline(&line, &cap, stdin);

        // 3. -1 means EOF (Ctrl-D) or a read error. Either way, stop. 
        if (nread == -1) {
            if (feof(stdin)) {
                printf("\n");
            } else {
                /* A real error, not just end of input */
                perror("getline");
            }
            break;
        }

        // 4. getline keeps the trailing '\n'. 
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        
        printf("You typed: \"%s\" (%zu chars)\n", line, strlen(line));
    }

    /* 5. Free the one buffer getline allocated. Without this, valgrind
        reports a leak. This runs on every exit path from the loop. */
    free(line);
    return 0;
}