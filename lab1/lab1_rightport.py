# Kirsten Mayland
# Lab1 - Computer Networks, Exercise 3, Part 3

# Credit for help: https://www.digitalocean.com/community/tutorials/how-to-use-netcat-to-establish-and-test-tcp-and-udp-connections
# Credit for help: https://www.geeksforgeeks.org/python-binding-and-listening-with-sockets/#
# Discussed with: Selena Han

import socket

bot_server = "129.170.212.8"    # from ifconfig, see eno2
bot_port = 603                  # from netstat -n -l

msg = bytes([9, 0, 255, ord('f'), ord('0'), ord('0'), ord('5'), ord('6'), ord('3'), ord('g')])

for source_port in range(2000, 3000): # valid ports 2000-2999
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # TCP
        # sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # UDP
        sock.bind(("0.0.0.0", source_port))     # 0.0.0.0 = eno2's broadcast

        sock.connect((bot_server, bot_port))
        sock.send(msg)

        response = sock.recv(1024).decode()

        if "Error: Incorrect source port." not in response:
            print("Found correct source port: ", source_port)
            print("Response:", response)
            sock.close()
            exit()
        else:
            sock.close()

    except Exception as e:
         print(f"Error '{e}' occurred when binding to source port {source_port}")

print("Rip try again, no valid source ports found")



# Note, to talk to bot use:
# cat /home/kirstenm/COURSES/CS60-Networks/lab1/lab1_rightport.py | ssh kirsten@thepond.cs.dartmouth.edu -p 106 python -