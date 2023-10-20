CFLAGS=-std=c11 -g -static

9cc: main.c parse.c codegen.c
	gcc -Wall -c main.c
	gcc -Wall -c parse.c
	gcc -Wall -c codegen.c
	gcc -o main.o parse.o codegen.o

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean