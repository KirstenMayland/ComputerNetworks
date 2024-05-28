# Kirsten Mayland
# Lab 3 Computer Networks
# Spring 2024
# Credit: Framework provided by ChatGPT

from scapy.all import *
from common import *


# Configuration
src_ip = '192.168.1.100'  # Replace with your source IP
dest_ip = '93.184.216.34'  # Example IP (example.com)
src_port = 12345
dest_port = 80
file_contents = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!"  # Example response

# ------------------------------packet_callback------------------------------
# Sniff TCP packets containing HTTP requests
def packet_callback(packet):
    if packet.haslayer(TCP) and packet[TCP].dport == dest_port:
        # Extract necessary details from the incoming packet
        ip = packet[IP]
        tcp = packet[TCP]
        seq = tcp.seq
        ack_seq = tcp.ack

        # Complete the three-way handshake
        # Send SYN-ACK
        flags = 0x12  # SYN-ACK
        syn_ack_packet = create_packet(dest_ip, ip.src, dest_port, tcp.sport, ack_seq, seq + 1, flags, socket.htons(5840))
        send(syn_ack_packet, verbose=0)

        # Wait for ACK
        time.sleep(0.5)

        # Send HTTP response
        flags = 0x18  # PSH-ACK
        response_packet = create_packet(dest_ip, ip.src, dest_port, tcp.sport, ack_seq, seq + 1, flags, socket.htons(5840), file_contents.encode())
        send(response_packet, verbose=0)

# Start sniffing
sniff(filter=f"tcp and dst port {dest_port}", prn=packet_callback, store=0)