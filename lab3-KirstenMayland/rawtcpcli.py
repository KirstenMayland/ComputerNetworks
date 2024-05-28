# Kirsten Mayland
# Lab 3 Computer Networks
# Spring 2024
# Credit: Framework provided by ChatGPT

from common import *
import select
import sys

# Configuration
src_ip = '10.132.55.174'  # 172.21.76.197
dest_ip = '129.170.212.8'  # packetbender.com
src_port = 12345
dest_port = 8080

# ------------------------------open raw socket------------------------------
try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_RAW)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
except socket.error as e:
    print(f"Socket creation failed: {e}")
    sys.exit(1)

# ------------------------------send SYN packet------------------------------
try:
    seq = 0
    ack_seq = 0
    flags = 2  # SYN
    window = socket.htons(5840)
    packet = create_packet(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window)
    sock.sendto(packet, (dest_ip, 0))
except Exception as e:
    print(f"Failed to send SYN packet: {e}")
    sock.close()
    sys.exit(1)

# ------------------------------receive SYN-ACK packet------------------------------
try:
    sock.settimeout(5)  
    while True:
        raw_packet = sock.recv(65535)
        ip_header = raw_packet[0:20]
        iph = struct.unpack('!BBHHHBBH4s4s', ip_header)
        ip_protocol = iph[6]
        if ip_protocol == socket.IPPROTO_TCP:
            tcp_header = raw_packet[20:40]
            tcph = struct.unpack('!HHLLBBHHH', tcp_header)
            if tcph[1] == src_port and tcph[0] == dest_port:
                seq = tcph[3]
                ack_seq = tcph[2] + 1
                break
except socket.timeout:
    print("No SYN-ACK received")
    sock.close()
    exit()
except Exception as e:
    print(f"Error receiving SYN-ACK packet: {e}")
    sock.close()
    sys.exit(1)

# ------------------------------send ACK packet------------------------------
flags = 16  # ACK
packet = create_packet(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window)
sock.sendto(packet, (dest_ip, 0))

# ------------------------------receive data------------------------------
# Set up to receive data
sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_TCP)
sock.bind((src_ip, src_port))

# Receive data
data_received = b''
while True:
    ready = select.select([sock], [], [], 5)
    if ready[0]:
        raw_packet = sock.recv(65535)
        ip_header = raw_packet[0:20]
        iph = struct.unpack('!BBHHHBBH4s4s', ip_header)
        ip_protocol = iph[6]
        if ip_protocol == socket.IPPROTO_TCP:
            tcp_header = raw_packet[20:40]
            tcph = struct.unpack('!HHLLBBHHH', tcp_header)
            if tcph[1] == src_port and tcph[0] == dest_port:
                data = raw_packet[40:]
                data_received += data
                # Send ACK for each received packet
                seq = tcph[3]
                ack_seq = tcph[2] + len(data)
                flags = 16  # ACK
                packet = create_packet(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window)
                sock.sendto(packet, (dest_ip, 0))
    else:
        break

# Save to file
with open('received_data.txt', 'wb') as f:
    f.write(data_received)

print("Data saved to received_data.txt")
sock.close()