all:	test

test:	unit7 Makefile
	./check_if_committed && ./unit7

unit7:	Makefile unit7.o unit7.h wu7_solution.c
	gcc -g -Wall -o unit7 unit7.o wu7_solution.c
