# msh: A Minimal Unix Shell

**Name:** Cynthia Chepkorir
**Student ID:** SCT212-0070/2024
**Course:** ICS 2305 Systems Programming, Practical Assignment (Question 1)

`msh` is a small command-line shell written in C. It reads a command, starts it as a separate process with `fork()`, `execvp()` and `waitpid()`, and reports what happened. It was written to show clearly how Unix runs programs.

## Build and run

Requirements: Linux (or WSL), `gcc` and `make`.

```bash
git clone https://github.com/Towett008/msh_SCT212_0070_2024
cd msh_SCT212_0070_2024
make
./msh
```

To remove the compiled program:

```bash
make clean
```



### Example session

```
msh[0]> echo hello world
hello world
msh[0]> cd /tmp
msh[0]> pwd
/tmp
msh[0]> echo hi > f.txt
msh[0]> cat f.txt
hi
msh[0]> sleep 3 &
[bg] started pid 4321
msh[0]> help
...
msh[0]> exit
```

The number in the prompt is the exit status of the previous command.

## Features implemented

| Task | Requirement | Status |
|------|-------------|--------|
| 1 | Prompt, read a line of any length with `getline()`, exit cleanly on Ctrl-D with no memory leaks | Complete |
| 2 | Split each line into `argv` on spaces and tabs, ignore blank lines and extra whitespace, end `argv` with `NULL` | Complete |
| 3 | Run external commands with `fork()`, `execvp()` and `waitpid()`, report errors, keep running after an unknown command | Complete |
| 4 | Built-ins `cd [dir]` (no argument goes to `$HOME`), `help` and `exit` | Complete |
| 5 | Show the exit status of the previous command in the prompt, such as `msh[0]>`, with `128 + signal number` for death by signal | Complete |
| 6 | Background execution with a trailing `&`, prompt returns at once and prints the PID, finished jobs are reaped so no zombies remain | Complete |
| 7 | Output redirection with `>` (create or truncate, mode 0644), done in the child process only | Complete |
| 8 | Ctrl-C interrupts a running foreground command but does not terminate the shell | Complete |

## Design explanation

msh is a loop: it prints the prompt, reads a line with `getline()`, splits it into an `argv` array with `strtok()`, and then either runs a built-in or starts an external program.

For an external program the shell uses the fork / exec / wait cycle. `fork()` makes a copy of the running shell, so there are two processes, and the only difference is the return value: 0 in the child and the child's PID in the parent. In the child I call `execvp()`, which replaces the copy with the requested program and searches `PATH` for it. If `execvp()` works it never returns; if it returns, the command could not be run, so the child prints an error and leaves with `_exit(127)` and the shell carries on. In the parent I call `waitpid()` on that PID, so the shell sleeps until the child finishes, retrying if a signal interrupts the wait. The status it collects is decoded with `WIFEXITED` and `WEXITSTATUS`, or `WIFSIGNALED` and 128 + the signal number, and shown in the prompt. I fork first and exec second because exec on its own would replace the shell with the command and the shell would disappear.

`cd` and `exit` are built-ins because they have to change the shell process itself. Each process has its own working directory, so a `chdir()` done in a child would be lost when that child exits, and an external `exit` program would only end its own process, not the shell. These two run directly inside the shell without `fork`. `help` is also built in.

A child that has finished stays a zombie until its parent collects its status with `waitpid()`. Foreground commands are waited for straight away, so they never become zombies. Background commands (ending in `&`) are not waited for, so I call `reap_children()` before every prompt and again after a line is read. It loops over `waitpid(-1, &status, WNOHANG)`, which collects every finished child but never blocks, so none are left behind.

## Why `cd` cannot be an external program

Every process has its own current working directory, and a child process gets only a copy of it from its parent. If `cd` were an external program, the shell would `fork()` a child and that child would call `chdir()`. The child's directory would change, but the child then exits, and the parent shell would still be in the old directory. Nothing would appear to happen. The only way to change the directory the shell works in is to call `chdir()` inside the shell process itself, so `cd` has to be a built-in.

This can be seen in msh. Run `pwd`, then `sh -c "cd /tmp"`, then `pwd` again. The second `pwd` shows the same folder as the first, because `cd` ran in a child that has already gone. The same reasoning applies to `exit`, which must end the shell and not just a child.



### Known limitations

- **No quoting or escaping:** Quotes are not parsed, so arguments cannot contain spaces (e.g., `echo "a b"` passes the literal quote characters to `echo`).
- **Limited I/O redirection and no pipes:** Only single output truncation (`>`) at the start of a word is supported; there is no append (`>>`), input redirection (`<`), `stderr` redirection (`2>`), or piping (`|`), and built-ins (`cd`, `help`) cannot be redirected.
- **No command chaining or multi-command lines:** Only one command can run per line (`;` and `&&` are unsupported), and `&` is only recognized at the very end of a command.
- **Deferred background zombie reaping & no job control:** Background jobs that finish while `msh` is waiting at `getline()` remain zombies until the next user input triggers `reap_children()`, and there is no `jobs`, `fg`, or `bg` support.
- **Limited signal handling:** Only `SIGINT` (`Ctrl-C`) is caught; `Ctrl-\` (`SIGQUIT`) and `Ctrl-Z` (`SIGTSTP`) retain their default behaviors and can terminate or suspend the shell.

## Sources and AI-tool declaration

**Sources consulted**
- Linux manual pages for how each call and its return values work.
- ICS 2305  notes 

**AI tool used**
- Claude was used as a tutor and coding assistant. It guided me through the project one task at a time, suggested and explained the code , it helped me understand the system calls. I built the project in my own repository, compiled and ran it, tested each task, and committed each task separately. 