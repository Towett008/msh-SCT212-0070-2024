/*
 * msh.c - a minimal Unix shell
 * Task 1: prompt, read a line of any length, exit cleanly on Ctrl-D
 * Task 2: split the line into an argv array
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* NEW: limits and separators for parsing */
#define MAX_ARGS 64          /* argv has room for 63 words + the NULL */
#define DELIMS   " \t"       /* a word is separated by spaces or tabs */

/* NEW: parse a line into an argv array, return argc */
static int parse_line(char *line, char **argv)
{
    int   argc  = 0;
    char *token = strtok(line, DELIMS);      /* first call: pass the string */

    /* Keep one slot free (MAX_ARGS - 1) for the terminating NULL. */
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, DELIMS);        /* later calls: pass NULL */
    }

    /* If a token is still left, the line had more words than we can hold. */
    if (token != NULL) {
        fprintf(stderr, "msh: too many arguments (max %d)\n", MAX_ARGS - 1);
        argv[0] = NULL;
        return -1;
    }

    argv[argc] = NULL;   /* execvp requires argv to end with NULL */
    return argc;
}

int main(void)
{
    char   *line = NULL;
    size_t  cap  = 0;
    ssize_t nread;
    char   *argv[MAX_ARGS];   /* NEW: array of pointers to the words */
    int     argc;             /* NEW: how many words we found */

    while (1) {
        printf("msh> ");
        fflush(stdout);

        nread = getline(&line, &cap, stdin);

        if (nread == -1) {
            if (feof(stdin)) {
                printf("\n");
            } else {
                perror("getline");
            }
            break;
        }

        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        /* NEW: split the line into words */
        argc = parse_line(line, argv);

        /* NEW: a blank line (0 words) or an error (-1): show a new prompt */
        if (argc <= 0) {
            continue;
        }

        /* TEMPORARY: print what we parsed so we can check Task 2.
         * Task 3 will replace this with running the command. */
        printf("argc = %d\n", argc);
        for (int i = 0; argv[i] != NULL; i++) {
            printf("  argv[%d] = \"%s\"\n", i, argv[i]);
        }
    }

    free(line);
    return 0;
}