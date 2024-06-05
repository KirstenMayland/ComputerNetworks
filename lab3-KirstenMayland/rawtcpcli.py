# Kirsten Mayland
# Lab 3 Computer Networks
# Spring 2024
# Credit: Framework provided by ChatGPT

from common import *
import sys
import random

# Configuration
RETRANSMISSION_TIMEOUT = 5
MAX_RETRANSMISSION_ATTEMPTS = 5

# ------------------------------send SYN packet------------------------------
def send_syn(seq):
    syn_packet = IP(src=CLI_IP, dst=SERV_IP) / TCP(sport=CLI_PORT, dport=SERV_PORT, flags="S", seq=seq)
    send(syn_packet, verbose=False)
    print(f"Sent SYN packet with seq={seq} to {SERV_IP}:{SERV_PORT}")

# ------------------------------process SYN-ACK and send ACK------------------------------
def process_syn_ack(packet):
    if packet.haslayer(TCP):
        print(f"flags = {packet[TCP].flags} --------")

    if packet.haslayer(TCP) and packet[TCP].flags == "SA":
        print("SYN-ACK received")
        tcp = packet[TCP]
        ack_packet = IP(src=CLI_IP, dst=SERV_IP) / TCP(sport=CLI_PORT, dport=SERV_PORT, flags="A", seq=tcp.ack, ack=tcp.seq + 1)
        send(ack_packet, verbose=False)
        print(f"Sent ACK packet with seq={tcp.ack}, ack={tcp.seq + 1} to {SERV_IP}:{SERV_PORT}")
        return True
    return False

# ------------------------------main------------------------------
def main():
    seq = random.randint(1000, 50000)  # Initial sequence number
    retransmission_attempts = 0
    syn_ack_received = False

    while retransmission_attempts < MAX_RETRANSMISSION_ATTEMPTS and not syn_ack_received:
        send_syn(seq)
        packets = sniff(filter=f"tcp and host {SERV_IP}", timeout=RETRANSMISSION_TIMEOUT)
        if packets:
            for pkt in packets:
                print(pkt.summary())  # Verbose output for captured packets
                if process_syn_ack(pkt):
                    syn_ack_received = True
                    break
            if not syn_ack_received:
                print("No valid SYN-ACK received, retransmitting SYN...")
        else:
            print("No SYN-ACK received, retransmitting SYN...")

        retransmission_attempts += 1

    if not syn_ack_received:
        print("Max retransmission attempts reached. Exiting.")
        sys.exit(1)

    print("Handshake completed")

    # Reciev data
    data_received = b''
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_TCP)
    recv_sock.bind((CLI_IP, CLI_PORT))

    while True:
        ready = select.select([recv_sock], [], [], 5)
        if ready[0]:
            raw_packet = recv_sock.recv(65535)
            # TODO: need to check if even received anything
            ip_header = raw_packet[0:20]
            iph = struct.unpack('!BBHHHBBH4s4s', ip_header)
            ip_protocol = iph[6]
            if ip_protocol == socket.IPPROTO_TCP:
                tcp_header = raw_packet[20:40]
                tcph = struct.unpack('!HHLLBBHHH', tcp_header)
                if tcph[1] == CLI_PORT and tcph[0] == SERV_PORT:
                    data = raw_packet[40:]
                    data_received += data
        else:
            break

    # Save to file
    with open('received_data.txt', 'wb') as f:
        f.write(data_received)

    print("Data saved to received_data.txt")

    # TODO: send FIN
    # TODO: wait for serv ACK
    # TODO: wait for serv FIN
    # TODO: wait for serv ACK


    # Close sockets
    recv_sock.close()

if __name__ == "__main__":
    main()