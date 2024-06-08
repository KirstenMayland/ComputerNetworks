# Final Project - Computer Networks 24S, Kirsten Mayland
### Authentication Across TCP/UDP Server Client Connections
This project is an extention of lab 2. Any errors with the base functionality of lab 2 are holdovers from that lab. This project strictly deals with the authentication aspect.

# Compiling
To compile all the necessary executables run `$ make all`. If you want to compile a specific executable you can designate it with `$ make <client/server>`. For example, `$ make tcpcli`, `$ make tcpserv`, or analogous. \
\
To remove all the executables, object files, and received gifs, run `$ make clean`.

If the SSL gives you issues when compiling, ensure that you have it downloaded:
```bash
$ sudo apt-get install openssl
$ sudo apt-get install libssl-dev
```

# Authentication
The new final project code is in `authentication.c` and `authentication.h`, in addition to lines overhauling `tcpcli.c`, `tcpserv.c`, `udpcli.c`, and `udpserv.c`

To get the servers' key and certificate:
```bash
# Generate private key
openssl genpkey -algorithm RSA -out server.key

# Generate a self-signed certificate
openssl req -new -x509 -key server.key -out server.crt -days 365
```

Note: TCP cli/serv is completely updated with SSL/TSL; UDP cli/serv is in the progress with DTSL. Current error is that SSL_connect in `udpcli.c` keeps returning -1.

My debugging so far has revealed that 1) I can talk across the internet using UDP with my current set up and that 2) :
```bash
SSL_connect:before SSL initialization
SSL_connect:error in SSLv3/TLS write client hello
UDP Client: Unable to connect to SSL: Destination address required
```