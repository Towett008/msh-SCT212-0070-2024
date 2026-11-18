CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

# Default target: build the shell
msh: msh.c
	$(CC) $(CFLAGS) -o msh msh.c

# Remove build output
clean:
	rm -f msh *.o

.PHONY: clean