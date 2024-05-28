# Kirsten Mayland
# Lab 3 Computer Networks
# Spring 2024
# Credit: Framework provided by ChatGPT

import socket
import struct
import time

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
    total_length = 0  # Kernel will fill the correct total length
    id = 54321
    flags_offset = 0
    ttl = 64
    protocol = socket.IPPROTO_TCP
    checksum = 0
    src_ip = socket.inet_aton(src_ip)
    dest_ip = socket.inet_aton(dest_ip)
    return struct.pack('!BBHHHBBH4s4s', version_ihl, tos, total_length, id, flags_offset, ttl, protocol, checksum, src_ip, dest_ip)

# ------------------------------create_tcp_header------------------------------
def create_tcp_header(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window):
    offset_res = (5 << 4) + 0
    urg_ptr = 0
    tcp_header = struct.pack('!HHLLBBHHH', src_port, dest_port, seq, ack_seq, offset_res, flags, window, 0, urg_ptr)
    pseudo_header = struct.pack('!4s4sBBH', socket.inet_aton(src_ip), socket.inet_aton(dest_ip), 0, socket.IPPROTO_TCP, len(tcp_header))
    tcp_checksum = checksum(pseudo_header + tcp_header)
    return struct.pack('!HHLLBBH', src_port, dest_port, seq, ack_seq, offset_res, flags, window) + struct.pack('H', tcp_checksum) + struct.pack('!H', urg_ptr)

# ------------------------------create_packet------------------------------
def create_packet(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window, payload=b''):
    ip_header = create_ip_header(src_ip, dest_ip)
    tcp_header = create_tcp_header(src_ip, dest_ip, src_port, dest_port, seq, ack_seq, flags, window)
    return ip_header + tcp_header + payload