# Kirsten Mayland
# Lab 3 Computer Networks
# Spring 2024
# Credit: Framework provided by ChatGPT

import socket
import struct
import time
import select
import sys

ip_string = '!BBHHHBBH4s4s'
tcp_string = '!HHLLBBHHH'

# Configuration
src_ip = '10.132.55.174'  # 172.21.76.197
dest_ip = '129.170.212.8'  # packetbender.com
src_port = 12345
dest_port = 8080

# Utility functions
# ------------------------------checksum------------------------------
def checksum(data):
    s = 0
    for i in range(0, len(data), 2):
        w = (data[i] << 8) + (data[i + 1] if i + 1 < len(data) else 0)
        s = s + w
    s = (s >> 16) + (s & 0xffff)
    s = ~s & 0xffff
    return s

# ------------------------------create_ip_header------------------------------
def create_ip_header(src_ip, dest_ip):
    version_ihl = (4 << 4) + 5
    tos = 0
    total_length = 0  # kernel will fill the correct total length
    id = 54321
    flags_offset = 0
    ttl = 64
    protocol = socket.IPPROTO_TCP
    checksum = 0
    src_ip = socket.inet_aton(src_ip)
    dest_ip = socket.inet_aton(dest_ip)
    return struct.pack(ip_string, version_ihl, tos, total_length, id, flags_offset, ttl, protocol, checksum, src_ip, dest_ip)

# ------------------------------create_tcp_header------------------------------
def create_tcp_header(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window):
    offset_res = (5 << 4) + 0
    urg_ptr = 0
    tcp_header = struct.pack(tcp_string, src_port, dest_port, seq, ack_seq, offset_res, flags, window, 0, urg_ptr)
    pseudo_header = struct.pack('!4s4sBBH', socket.inet_aton(src_ip), socket.inet_aton(dest_ip), 0, socket.IPPROTO_TCP, len(tcp_header))
    tcp_checksum = checksum(pseudo_header + tcp_header)
    return struct.pack('!HHLLBBH', src_port, dest_port, seq, ack_seq, offset_res, flags, window) + struct.pack('H', tcp_checksum) + struct.pack('!H', urg_ptr)

# ------------------------------create_packet------------------------------
def create_packet(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window):
    ip_header = create_ip_header(src_ip, dest_ip)
    tcp_header = create_tcp_header(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window)
    return ip_header + tcp_header

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
        iph = struct.unpack(ip_string, ip_header)
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
        iph = struct.unpack(ip_string, ip_header)
        ip_protocol = iph[6]
        if ip_protocol == socket.IPPROTO_TCP:
            tcp_header = raw_packet[20:40]
            tcph = struct.unpack(tcp_string, tcp_header)
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