CFLAGS = -Wall -pedantic -std=c11 -ggdb
VFLAGS = --leak-check=full --show-leak-kinds=all
CC = gcc

# -------------------------- #

all: clean tcpcli

tcpserv: tcpserv.c
	$(CC) $(CFLAGS) -o $@ $<

tcpcli: customProtocolClient.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm tcpcli
