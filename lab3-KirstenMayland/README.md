# Lab 3 - Computer Networks 24S, Kirsten Mayland
### Emulating TCP server and client connections

# NOTE ON CONNECTIONS:
To get the below client and server to talk to each other, go to `common.py` and configure the correct client/server IP addresses/ports.

# Task 1 - Emulate the server side of a TCP connection

To run my created TCP server, run `$ python3 rawtcpserv.py` in another VM that you can talk to. In this case, I ran `rawtcpserv.py` on thepond. 

# Task 2 - Emulating a TCP client connection

To run my created TCP client, run `$ sudo python3 rawtcpcli.py` on your computer.

# Completion
Correctly formatted packets are currently being sent back and forth to some degree of success as you can see on Wireshark. Fully functionality hasn't been reached yet 

Currently: cli-sends SYN, serv-receives SYN, serv-sends SYN-ACK, cli-receives the SYN-ACK, cli-sends ACK (?), serv-doesn't process ACK (treats as 2nd SYN), serv-send data, cli-receives data
