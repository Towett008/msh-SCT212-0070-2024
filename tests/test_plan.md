## Test environment

- Operating system: WSL (Ubuntu) on Windows
- Compiler: gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- Build command: `make`


## Required tests

| Input | Expected behaviour | Actual result |
|-------|--------------------|---------------|
| (blank line) | New prompt, no error | A new prompt appeared immediately with no output and no error. |
| `echo hello world` | Prints text; status 0 | Printed `hello world`. The next prompt was `msh[0]>`. |
| `ls /nonexistent` | Error from `ls`; non-zero status | `ls` printed `ls: cannot access '/nonexistent': No such file or directory`. The next prompt was `msh[2]>`  |
| `nosuchcommand` | Error message; status 127 | Printed `msh: nosuchcommand: No such file or directory`. The next prompt was `msh[127]>` |
| `cd /tmp` then `pwd` | Prints `/tmp` | `pwd`  printed `/tmp`. |
| `sleep 3 &` | Prompt returns at once; PID shown | Printed `[bg] started pid 74402` and the prompt `msh[0]>` came back immediately. About 3 seconds later, pressing Enter showed `[done] pid 74402 finished with status 0`. |
| `echo hi > f.txt` then `cat f.txt` | File contains `hi` |  `cat f.txt` printed `hi` on the screen. |
| `sleep 100` then Ctrl-C | Sleep stops; shell survives; status 130 | After pressing Ctrl-C, `sleep` stopped and a new prompt appeared as `msh[130]>` |
| Ctrl-D at the prompt | Shell exits cleanly | A newline was printed and the shell exited, returning to the normal WSL prompt. |

## My own tests

| Input | Expected behaviour | Actual result |
|-------|--------------------|---------------|
| `false` then `true` | Prompt shows `msh[1]>` after `false`, then `msh[0]>` after `true` | After `false` the prompt was `msh[1]>`. After `true` it was `msh[0]>`. |
| `cd` (no argument) then `pwd` | Goes to the home directory (`$HOME`) | `cd` printed nothing, and `pwd` printed `/home/administrator`. |
| `cd /nonexistent` | Error message from `cd`; status 1; shell keeps running | Printed `msh: cd: /nonexistent: No such file or directory`. The prompt became `msh[1]>` and the shell kept running. |
| `echo again > f.txt` then `cat f.txt` (after the earlier `f.txt` test) | File now contains only `again`, so the old content was truncated | `cat f.txt` printed only `again`. The earlier `hi` was gone. |
| `sleep 3 &`, wait 4 seconds, then press Enter | `[done] pid ... finished with status 0` is shown and no `<defunct>` process remains | Pressing Enter after the wait printed `[done] pid 75425 finished with status 0`.  |
| `sleep 60`, then `pkill -TERM sleep` from a second terminal | Prompt shows `msh[143]>` (128 + 15) | After running `pkill -TERM sleep` in the second terminal, the msh prompt became `msh[143]>`. |



