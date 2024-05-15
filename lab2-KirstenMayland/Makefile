# Kirsten Mayland
# Makefile for Lab 2, Computer Networks 24S

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb
VFLAGS = --leak-check=full --show-leak-kinds=all

OBJS_S = database.o
OBJS_C = client.o

# -------------------------- #


all: tcpcli tcpserv udpcli udpserv tcpcli2 tcpserv2 udpcli2 udpserv2

tcpcli: tcpcli.c $(OBJS_C)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_C)

tcpcli2: tcpcli2.c $(OBJS_C)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_C)

tcpserv: tcpserv.o $(OBJS_S)
	$(CC) $(CFLAGS) -o tcpserv tcpserv.o $(OBJS_S)

tcpserv2: tcpserv2.o $(OBJS_S)
	$(CC) $(CFLAGS) -o tcpserv2 tcpserv2.o $(OBJS_S)

udpcli: udpcli.c $(OBJS_C)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_C)

udpcli2: udpcli2.c $(OBJS_C)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS_C)

udpserv: udpserv.o $(OBJS_S)
	$(CC) $(CFLAGS) -o udpserv udpserv.o $(OBJS_S)

udpserv2: udpserv2.o $(OBJS_S)
	$(CC) $(CFLAGS) -o udpserv2 udpserv2.o $(OBJS_S)

clean:
	rm tcpcli tcpserv udpcli udpserv tcpcli2 tcpserv2 udpcli2 udpserv2 *.gif *.o
