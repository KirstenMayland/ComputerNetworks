
Silly TCP server that takes an integer and a power and raises the number to that power,
and a client to match:

Request: <power: unit8> <operand: uint32>

Response: <status: uint8> <result: uint32> <len: uint16> <result_as_string: char[len]>

-------------------[ From the client side ]-------------------

$ ./tcpcli 9 2
Result: 81
81

$ ./tcpcli 9 3
Result: 729
729

nc + xxd show what the server sends back. Note -e and -n options on echo:

$ echo -ne '\x02\x00\x00\x00\x09' | nc 127.0.0.1 5051 | xxd
0000000: 0100 0000 5100 0238 31                   ....Q..81

-------------------[ From the server side: ]-------------------

# ./tcpserv 
server: can't bind local address: Address already in use

Oops, another process is listening on this port (5051). It's probably
a version of the same server that I left running. Yes, indeed:

# ps ax | grep serv
<skipped>
11665 s018  S+     0:00.00 ./tcpserv
11670 s018  Z+     0:00.00 (tcpserv)
11672 s018  Z+     0:00.00 (tcpserv)
11675 s018  Z+     0:00.00 (tcpserv)
11677 s018  Z+     0:00.00 (tcpserv)
11679 s018  Z+     0:00.00 (tcpserv)
11687 s018  Z+     0:00.00 (tcpserv)
11689 s018  Z+     0:00.00 (tcpserv)
11691 s018  Z+     0:00.00 (tcpserv)

(Showing up in parens are the defunct children of the parent process.)
(Since the parent didn't collect their exist status after they exited,
 they still clutter up the process table; they are called "zombies".)

So kill them all:

# killall tcpserv

and restart (setsockopt takes care of immediately reusing the port):

# ./tcpserv 
In child, calling process_request
read 5 bytes
Got num 9 pow 2
Sending back result 81
read 0 bytes
In child, calling process_request
read 5 bytes
Got num 9 pow 3
Sending back result 729
read 0 bytes

(Input too short: I sent only 4 bytes, not 5)

In child, calling process_request
read 4 bytes
process_request: truncated inputIn child, calling process_request
read 5 bytes
Got num 9 pow 2
Sending back result 81
read 0 bytes
^C

These should show bytes sent by the clients, except xxd's buffering
becomes a problem:

# nc -l 5051 | xxd
^C
# nc -l 127.0.0.1 5051 | xxd
^C

But something is getting printed!

# nc -l 127.0.0.1 5051 
          ^C
^^^^^^^^^^

So redirect to a file, then use xxd:

# nc -l 127.0.0.1 5051 > req

# ls -l req
-rw-r--r-- 1 root staff 5 Apr 11 18:09 req

# xxd req 
0000000: 0300 0000 09                             .....

Now I removed one of the htonl's from the client code
where I build my request, req.num = num instead of req.num = htonl(num) .
Immediately, this bug shows up: (guess what 0x9000000 is):

# ./tcpserv 
In child, calling process_request
read 5 bytes
Got num 150994944 pow 2
Sending back result 0
read 0 bytes
In child, calling process_request
read 5 bytes
Got num 9 pow 2
Sending back result 81
read 0 bytes
^C

Remove other ntohl and htonl's, and a ntohs from client,
and see what happens. Understand why.

  