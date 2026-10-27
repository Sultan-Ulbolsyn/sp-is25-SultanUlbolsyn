
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread

all: procinfo topmem

procinfo: procinfo.c
	$(CC) $(CFLAGS) procinfo.c -o procinfo

topmem: topmem.c
	$(CC) $(CFLAGS) topmem.c -o topmem

clean:
	rm -f procinfo topmem
