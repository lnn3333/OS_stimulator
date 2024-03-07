FLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror

all: build

build: main.o list.o help.o
	gcc $(FLAGS) main.o list.o help.o -o sim

main.o: main.c main.h
	gcc $(FLAGS) -c main.c

list.o: list.c list.h
	gcc $(FLAGS) -c list.c

help.o: help.c main.h structure.h
	gcc $(FLAGS) -c help.c

run: build
	./sim

valgrind: build
	valgrind --leak-check=full ./sim

clean:
	rm -f sim *.o
