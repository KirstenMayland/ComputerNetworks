# Kirsten Mayland
# Makefile for Lab 2, Computer Networks 24S

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
VFLAGS = --leak-check=full --show-leak-kinds=all

OBJS = database.o

# -------------------------- #


all: tcpcli tcpserv

tcpserv: tcpserv.o $(OBJS)
	$(CC) $(CFLAGS) -o tcpserv tcpserv.o $(OBJS)

tcpcli: tcpcli.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm tcpcli tcpserv *.gif *.o
