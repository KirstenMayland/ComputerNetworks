# Kirsten Mayland
# Lab1 - Computer Networks, Exercise 3, Part 2

# Credit for help: https://www.geeksforgeeks.org/python-program-that-sends-and-recieves-message-from-client/#
# Discussed with: Selena Han

import socket

# create a socket at client side using TCP / IP protocol
# and connect it to server and port number on local computer
bot_server = "129.170.212.8"    # from ifconfig, see eno2
bot_port = 603                  # from netstat -n -l
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
sock.connect((bot_server, bot_port))


error_msg = None
msg_array = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]   # initial inquires to the bot showed it wants a message of 10 bytes

# loop through possible btye options of at index i
for i in range(256): 
    msg_array[0] = i  # HARDCODED: manually move i according to error message after correct current i byte is found

    # send message to bot and receive response
    msg = bytes(msg_array)
    sock.send(msg)
    response = sock.recv(1024).decode('UTF-8')

    # store initial error message 
    if error_msg == None:
        error_msg = response
        print(error_msg)

    # stop and reveal progress when new message is found
    elif response != error_msg:
        print("!!! A new byte made progress, i =: ", i)
        print("New error message: ", response)
        break


sock.close()



# History of byte discovery:
# [0, 0, 0, 0, 0, 0, 0, 0, 0, 0] Error: Message does not start with magic byte
# [9, 0, 0, 0, 0, 0, 0, 0, 0, 0] Error: Message does not start with magic 3 bytes
#  i^
# [9, 0, 255, 0, 0, 0, 0, 0, 0, 0] Error: Message payload does not contain valid net_id
#         i^
# [9, 0, 255, ord('f'), ord('0'), ord('0'), ord('5'), ord('6'), ord('3'), ord('g')] Error: Incorrect source port.

# Note: Tried to do a fully brute force method, but the error messages made it more annoying/more work than this method

# Note, to talk to bot use:
# cat /home/kirstenm/COURSES/CS60-Networks/lab1/lab1_rightmessage.py | ssh kirsten@thepond.cs.dartmouth.edu -p 106 python -