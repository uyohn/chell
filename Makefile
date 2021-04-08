# -*- MakeFile -*-

#target: dependencies
#	action

all: main

main: main.o chell.o
	gcc main.o chell.o -o main

main.o: main.c
	gcc -c main.c

chell.o: chell.c chell.h
	gcc -c chell.c

clean:
	rm -f *.o main
