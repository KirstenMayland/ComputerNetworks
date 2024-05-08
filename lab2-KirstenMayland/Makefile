# Kirsten Mayland
# Makefile for Lab 2, Computer Networks 24S

CFLAGS = -Wall -pedantic -std=c11 -ggdb
VFLAGS = --leak-check=full --show-leak-kinds=all
CC = gcc

# -------------------------- #

all: tcpcli tcpserv

remake: clean tcpcli tcpserv

tcpserv: tcpserv.c
	$(CC) $(CFLAGS) -o $@ $<

tcpcli: tcpcli.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm tcpcli tcpserv *.gif
