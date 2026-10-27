
CC = gcc
CFLAGS = -O0 -Wall -Wextra -std=c11 -pthread

all: race race_fixed philosophers deadlock

race: race.c
	$(CC) $(CFLAGS) race.c -o race

race_fixed: race_fixed.c
	$(CC) $(CFLAGS) race_fixed.c -o race_fixed

philosophers: philosophers.c
	$(CC) $(CFLAGS) philosophers.c -o philosophers

deadlock: deadlock.c
	$(CC) $(CFLAGS) deadlock.c -o deadlock

clean:
	rm -f race race_fixed philosophers deadlock
