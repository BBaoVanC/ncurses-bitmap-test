main: main.c
	cc -o main main.c -lncurses -Wall

run: main
	./main

.PHONY: run
