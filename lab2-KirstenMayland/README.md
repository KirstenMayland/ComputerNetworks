# Lab 2 - Computer Networks 24S, Kirsten Mayland
### A Custom TCP Protocol Client and Server via Berkeley Sockets

# Compiling
To compile all the necessary executables run `$ make all`. If you want to compile a specific executable you can designate it with `$ make tcpcli`, `$ make tcpserv`, or analogous. \
\
To remove all the executables, object files, and received gifs, run `$ make clean`.

# Exercise 1 - Custom Protocol _Client_ via Berkeley sockets

To run my created TCP Client, run `$ ./tcpcli <statecode> <opcode>` where 'state code' is valid two letter state code (eg. pa, Wi, NH) and 'opcode' is an integer from 1 to 5 inclusive. packetbender.com should then return data corresponding to the request. The following opcodes correspond to the different requests:
* 1 for the name of the state
* 2 for its capital
* 3 for the date when it officially became a state,
* 4 for the state motto
* 5 for the state flag (as a GIF)

For Exercise 1, `tcpcli` is mean to talk to packetbender.com. In order to ensure that it is doing so and not talking to another server, please make sure the `SERV_TCP_PORT` and `SERV_HOST_ADDR` are configured as below. You will need to go into `tcpcli.c`, change this, and recompile the code if not.
```c
// #define SERV_TCP_PORT   8901 // thepond
// #define SERV_HOST_ADDR  "129.170.212.8" // thepond

#define SERV_TCP_PORT   5050    // packetbender.com
#define SERV_HOST_ADDR  "71.19.146.5"   // packetbender.com
```
# Exercise 2 - Custom Protocol _Server_ via Berkeley sockets
## Task 1
Task 1 of Exercise 2 is run similar to Exercise 2, except you are talking to my created TCP Server. Firstly, ensure that `tcpcli` is talking to the created server on the pond not packetbender.com as seen below. You will need to go into `tcpcli.c`, change this, and recompile the code if not.
```c
#define SERV_TCP_PORT   8901 // thepond
#define SERV_HOST_ADDR  "129.170.212.8" // thepond

// #define SERV_TCP_PORT   5050    // packetbender.com
// #define SERV_HOST_ADDR  "71.19.146.5"   // packetbender.com
```
To run both programs, run `tcpserv` on thepond and `tcpcli` on your local computer. \
\
The usage of `tcpserv` is `$ ./tcpserv <port to listen on>`. The port number should be above 1024 and correspond to the `SERV_TCP_PORT` listed in `tcpcli.c`. In this case, the port number is set to 8901 so you would run `$ ./tcpserv 8901` on the pond. Once you have the server running, you can run `$ ./tcpcli <statecode> <opcode>` on your local computer and receive the requested data.

## Task 2
Task 2 of Exercise 2 involves creating a UDP client and server that talk to each other with the same protocol as the TCP client and server above. Again, for `./udpserv` ensure that the port number is above 1024 and corresponds to the `SERV_UDP_PORT` listed in `udpcli.c`.

Just like `tcpcli` and `tcpserv`, run `./udpcli <statecode> <opcode>` and `./udpserv <port to listen on>` on your computer and the pond respectively.

## Task 3
In progress

# Overview
### On memory:
I did my best to free malloc'd data as I went but I have not run valgrind on any of this code and there might be many memory leaks that I've missed

### On thepond:
The client-server interactions have been designed to run on the Dartmouth CS pond; however, they don't technically need to do so. Simply run the client and server on different Linux or MacOS systems that are able to talk to each other across the internet and it should work.

### For personal reference to submit:
```bash
scp -P 106 -r ../lab2-KirstenMayland kirsten@thepond.cs.dartmouth.edu:submissions/lab2
```