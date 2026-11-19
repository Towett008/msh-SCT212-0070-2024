// msh.c - a minimal Unix shell
// Task 1: prompt, read a line of any length, exit cleanly on Ctrl-D
// Task 2: split the line into an argv array
// Task 3: run external commands with fork, execvp and waitpid
// Task 4: built-in commands cd, help and exit

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 64      // argv holds up to 63 words plus the NULL
#define DELIMS   " \t"   // words are separated by spaces or tabs

// split line into words and store them in argv (changes line in place)
// returns the word count, 0 for a blank line, or -1 if there are too many words
static int parse_line(char *line, char **argv)
{
    int   argc  = 0;
    char *token = strtok(line, DELIMS);

    // stop at MAX_ARGS - 1 so there is always room for the NULL
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, DELIMS);
    }

    // a leftover token means the line had too many words
    if (token != NULL) {
        fprintf(stderr, "msh: too many arguments (max %d)\n", MAX_ARGS - 1);
        argv[0] = NULL;
        return -1;
    }

    argv[argc] = NULL;   // execvp needs argv to end with NULL
    return argc;
}

// NEW: change the shell's own directory (this is why cd must be a built-in)
static void builtin_cd(char **argv, int argc)
{
    const char *dir;

    // cd takes at most one argument
    if (argc > 2) {
        fprintf(stderr, "msh: cd: too many arguments\n");
        return;
    }

    if (argc == 1) {
        // no argument, so go to the home directory
        dir = getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "msh: cd: HOME not set\n");
            return;
        }
    } else {
        dir = argv[1];
    }

    // chdir returns -1 on failure, for example if the folder does not exist
    if (chdir(dir) == -1) {
        fprintf(stderr, "msh: cd: %s: ", dir);
        perror("");
    }
}

// NEW: print the list of built-in commands
static void builtin_help(void)
{
    printf("msh - a minimal shell\n");
    printf("Built-in commands:\n");
    printf("  cd [dir]   change directory (no argument goes to $HOME)\n");
    printf("  help       show this message\n");
    printf("  exit       leave the shell\n");
    printf("Any other command is run as an external program.\n");
}

// NEW: run cd or help if that is what the user typed
// returns 1 if it was a built-in, 0 if it was not (exit is handled in main)
static int try_builtin(char **argv, int argc)
{
    if (strcmp(argv[0], "cd") == 0) {
        builtin_cd(argv, argc);
        return 1;
    }
    if (strcmp(argv[0], "help") == 0) {
        builtin_help();
        return 1;
    }
    return 0;
}

// run argv as an external program and wait for it to finish
static void run_command(char **argv)
{
    int   status;
    pid_t pid;

    // flush stdout so buffered text is not copied into the child and printed twice
    fflush(stdout);

    pid = fork();   // returns 0 in the child and the child's PID in the parent

    if (pid < 0) {
        // fork failed, so report it and go back to the prompt
        perror("msh: fork");
        return;
    }

    if (pid == 0) {
        // child: replace this process with the command (searches $PATH)
        execvp(argv[0], argv);

        // execvp only returns if it failed
        fprintf(stderr, "msh: %s: ", argv[0]);
        perror("");

        // _exit skips stdio cleanup of the copied buffers, 127 means command not found
        _exit(127);
    }

    // parent: wait for the child, and retry if a signal interrupts the wait
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
                printf("\n");   // Ctrl-D, so move to a fresh line
            } else {
                perror("getline");
            }
            break;
        }

        // remove the newline that getline keeps
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        argc = parse_line(line, argv);

        // blank line or parse error, so just show a new prompt
        if (argc <= 0) {
            continue;
        }

        // NEW: exit leaves the loop so line gets freed below
        if (strcmp(argv[0], "exit") == 0) {
            break;
        }

        // NEW: run cd or help inside the shell, with no fork
        if (try_builtin(argv, argc)) {
            continue;
        }

        // not a built-in, so run it as an external command
        run_command(argv);
    }

    free(line);
    return 0;
}