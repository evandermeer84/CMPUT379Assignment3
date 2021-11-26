CC = gcc
CFLAGS = -Wall -O2  -O

all: tands.o client server 
	rm *.log

server:  tands.o server.o
	$(CC) -Wall -O2  -o  server server.o tands.o

client: tands.o client.o
	$(CC) -Wall -O2 -o client client.o tands.o
 
cliento: tands.o client.o
	$(CC) $(CFLAGS) -c client.c -o client.o

tands:
	$(CC) $(CFLAGS) -c tands.c -o tands.o

servero: tands.o
	$(CC) $(CFLAGS) -c server.c -o tands.o

clean:
	rm *.log

compress:
	zip evandermprodcon.zip main.c main.h tands.c Makefile

#Need to stop if from running everytime you go make
