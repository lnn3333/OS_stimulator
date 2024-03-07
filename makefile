FLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror

all: build

build: main.o
	gcc $(FLAGS) main.c list.o -o main 

run: build
	./main

valgrind: build
	valgrind --leak-check=full ./main

clean: 
	rm -f main

debug: 
	gdb main
