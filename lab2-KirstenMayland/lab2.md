# Lab 2 - Computer Networks 24S, Kirsten Mayland
### A Custom TCP Protocol Client and Server via Berkeley Sockets

# Exercise 1 - Custom protocol _client_ via Berkeley sockets

Use `./tcpcli` as standard --- finally works, currently set to talk to tcpserv but if you want it to talk to packetbender.com just switch the port and host address it talks to

# Exercise 2 - Custom protocol _server_ via Berkeley sockets
## Task 1
Use `./tcpserv` as standard -- finally works, shouldn't break but doesn't have robust error responses

## Task 2
Similar to tcpcli/serv, run `./udpcli` and `./udpserv` on your computer and the pong respectively

# Overview
### On memory:
I did my best to free malloc'd data as I went but I have not run valgrind on any of this code and there might be many memory leaks that I've missed

### For personal reference to submit:
```bash
scp -P 106 -r ../lab2-KirstenMayland kirsten@thepond.cs.dartmouth.edu:submissions/lab2
```