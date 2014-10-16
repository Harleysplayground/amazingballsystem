EXE = lab4

CC = gcc
CFLAGS = -c -g -Wall -Werror
LDFLAGS = -lrt

all : lab4

lab4 : linuxanalog.o lab4.o
	$(CC) $(LDFLAGS) linuxanalog.o lab4.o -o lab4

linuxanalog.o : linuxanalog.c
	$(CC) $(CFLAGS) linuxanalog.c

lab4.o : lab4.c
	$(CC) $(CFLAGS) lab4.c

clean :
	rm -rf *.o $(EXE)
