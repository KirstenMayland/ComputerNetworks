# Kirsten Mayland
# Makefile for Lab 2, Computer Networks 24S

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
VFLAGS = --leak-check=full --show-leak-kinds=all

OBJS_S = database.o authentication.o
OBJS_C = client.o authentication.o

# -------------------------- #


all: tcpcli tcpserv udpcli udpserv

tcpcli: tcpcli.c $(OBJS_C)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_C) -lssl -lcrypto

tcpserv: tcpserv.o $(OBJS_S)
	$(CC) $(CFLAGS) -o tcpserv tcpserv.o $(OBJS_S) -lssl -lcrypto

udpcli: udpcli.c $(OBJS_C)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_C) -lssl -lcrypto

udpserv: udpserv.o $(OBJS_S)
	$(CC) $(CFLAGS) -o udpserv udpserv.o $(OBJS_S) -lssl -lcrypto

clean:
	rm tcpcli tcpserv udpcli udpserv *.gif *.o
