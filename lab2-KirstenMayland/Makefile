# Kirsten Mayland
# Makefile for Lab 2, Computer Networks 24S

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
VFLAGS = --leak-check=full --show-leak-kinds=all

OBJS_S = database.o

# -------------------------- #


all: tcpcli tcpserv udpcli udpserv

tcpcli: tcpcli.c
	$(CC) $(CFLAGS) -o $@ $<

tcpserv: tcpserv.o $(OBJS_S)
	$(CC) $(CFLAGS) -o tcpserv tcpserv.o $(OBJS_S)

udpcli: udpcli.c
	$(CC) $(CFLAGS) -o $@ $<

udpserv: udpserv.o $(OBJS_S)
	$(CC) $(CFLAGS) -o udpserv udpserv.o $(OBJS_S)

clean:
	rm tcpcli tcpserv udpcli udpserv *.gif *.o
