
all:
	gcc -std=c99 -g cliconnect.c -lncurses -ljack -o cliconnect
