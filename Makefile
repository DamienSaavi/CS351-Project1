CC=g++
CFLAGS=-c -Wall
DEPS = msg.h

all: sender_receiver

sender_receiver: sender.o recv.o
	$(CC) $(CFLAGS)  sender.o recv.o -o sender_receiver

sender.o: sender.cpp
	$(CC) $(CFLAGS) -c sender.cpp

recv.o: recv.cpp
	$(CC) $(CFLAGS) -c recv.cpp

clean:
	rm -rf *o sender_receiver

