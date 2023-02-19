main: main.c
	cc -o main main.c -lncurses -Wall

run: main
	./main

clean:
	rm -f main

.PHONY: run clean
