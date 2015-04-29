CC = gcc
CFLAGS = -g -O2
OBJECTS = main.o IPC.o elevator.o  Users.o

elevator : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o elevator

main.o : main.c
	$(CC) $(CFLAGS) -c main.c

IPC.o : IPC.c
	$(CC) $(CFLAGS) -c IPC.c

elevator.o : elevator.c
	$(CC) $(CFLAGS) -c elevator.c

Users.o : Users.c
	$(CC) $(CFLAGS) -c Users.c

clean:				# Cleaning Object Files and Executable
	rm -rf *.o elevator

