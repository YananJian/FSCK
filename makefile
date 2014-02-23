CC = gcc
CFLAGS=-Wall

all: myfsck
myfsck: myfsck.o
myfsck.o: myfsck.c

run: myfsck
	./myfsck


