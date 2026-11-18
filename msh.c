/*
 * msh.c - a minimal Unix shell
 * Task 1: prompt, read a line of any length, exit cleanly on Ctrl-D
 * Task 2: split the line into an argv array
 * Task 3: run external commands with fork, execvp and waitpid
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>      
#include <unistd.h>     
#include <sys/types.h>   
#include <sys/wait.h>  

#define MAX_ARGS 64      /* argv holds up to 63 words plus the NULL */
#define DELIMS   " \t"   /* words are separated by spaces or tabs */

/*
 * parse_line - split line into words stored in argv (modifies line in place).
 * Returns the word count, 0 for a blank line, or -1 if there are too many words.
 */
static int parse_line(char *line, char **argv)
{
    int   argc  = 0;
    char *token = strtok(line, DELIMS);

    /* Stop at MAX_ARGS - 1 so there is always room for the NULL. */
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, DELIMS);
    }

    /* A leftover token means the line had too many words. */
    if (token != NULL) {
        fprintf(stderr, "msh: too many arguments (max %d)\n", MAX_ARGS - 1);
        argv[0] = NULL;
        return -1;
    }

    argv[argc] = NULL;   /* execvp needs argv to end with NULL */
    return argc;
}

/*
 * NEW: run_command - run argv as an external program and wait for it.
 * The shell stays alive because only the forked child becomes the new program.
 */
static void run_command(char **argv)
{
    int   status;
    pid_t pid;

    /* Flush stdout so buffered text is not copied into the child and printed twice. */
    fflush(stdout);

    pid = fork();   /* create the child: returns 0 in the child, child's PID in the parent */

    if (pid < 0) {
        /* fork failed, so report it and return to the prompt. */
        perror("msh: fork");
        return;
    }

    if (pid == 0) {
        /* CHILD: replace this process with the command, searching $PATH. */
        execvp(argv[0], argv);

        /* execvp only returns on failure, so report it. */
        fprintf(stderr, "msh: %s: ", argv[0]);
        perror("");

        /* _exit skips stdio cleanup of the copied buffers; 127 means "command not found". */
        _exit(127);
    }

    /* PARENT: wait for this child; retry if a signal interrupts the wait. */
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("msh: waitpid");
            break;
        }
    }
}

int main(void)
{
    char   *line = NULL;
    size_t  cap  = 0;
    ssize_t nread;
    char   *argv[MAX_ARGS];
    int     argc;

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

        /* Strip the trailing newline that getline keeps. */
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        argc = parse_line(line, argv);

        /* Blank line or parse error: just show a new prompt. */
        if (argc <= 0) {
            continue;
        }

        /* NEW: run the command (replaces the temporary argv printing). */
        run_command(argv);
    }

    free(line);
    return 0;
}