FLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: build

build: main.o list.o
	gcc $(FLAGS) main.o list.o -o sim

main.o: main.c
	gcc $(FLAGS) -c main.c

list.o: list.c
	gcc $(FLAGS) -c list.c

run: build
	./sim

valgrind: build
	valgrind --leak-check=full ./sim

clean:
	rm -f sim *.o
