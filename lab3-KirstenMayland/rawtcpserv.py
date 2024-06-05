# Kirsten Mayland
# Lab 3 Computer Networks
# Spring 2024
# Credit: Framework provided by ChatGPT

from common import *

file_contents = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!"  # Example response

# ------------------------------open raw socket------------------------------
try:
    sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_RAW)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)
except socket.error as e:
    print(f"Socket creation failed: {e}")
    sys.exit(1)
print("Opened raw socket...")

# ------------------------------send SYN-ACK packet------------------------------
def send_syn_ack(server_seq, client_seq):
    # NOTE: idk why every time I send a SYN-ACK, it turns into a RST-ACK; maybe it's bc I'm home and working through the VPN? idk
    syn_ack_packet = IP(src=SERV_IP, dst=CLI_IP) / TCP(sport=SERV_PORT, dport=CLI_PORT, flags="SA", seq=server_seq, ack=client_seq + 1)
    send(syn_ack_packet, verbose=False)
    print(f"Sent SYN-ACK packet with seq={server_seq}, ack={client_seq + 1} to {CLI_IP}:{CLI_PORT}")

# ------------------------------process ACK------------------------------
def process_ack(packet):
    print("checking if ACK")
    if packet.haslayer(TCP) and packet[TCP].flags == "A" and packet[TCP].dport == SERV_PORT:
        print("ACK received")
        return True
    return False

# ------------------------------packet_callback------------------------------
def packet_callback(packet):
    if packet.haslayer(TCP) and packet[TCP].flags == "S":
        # Extract necessary details from the incoming packet
        print(packet.summary())  # Verbose output for captured packets
        ip = packet[IP]
        tcp = packet[TCP]
        seq = tcp.seq
        ack_seq = tcp.ack

        # send SYN-ACK
        serv_seq = random.randint(1000, 50000)  # Initial sequence number
        send_syn_ack(serv_seq, seq)

        # wait for ACK
        ack_received = False
        packets = sniff(filter=f"tcp and host {CLI_IP}", timeout=RETRANSMISSION_TIMEOUT)
        if packets:
            for pkt in packets:
                print(pkt.summary())  # Verbose output for captured packets
                if process_ack(pkt):
                    ack_received = True

        if not ack_received:
            print("No ACK received, waiting for new SYN")
            return

        print("Handshake completed")


        # Send HTTP response
        flags = 0x18  # PSH-ACK
        response_packet = create_packet(SERV_IP, ip.src, SERV_PORT, tcp.sport, ack_seq, seq + 1, flags, socket.htons(5840), file_contents.encode())
        sock.sendto(response_packet, (CLI_IP, 0))
        # send(response_packet, verbose=0)
        print(f"Sent back HTTP response to packet...")


        # TODO: FIN-ACK closer

# ------------------------------sniff tcp stream for SYNs------------------------------
print("Starting to sniff...")
sniff(filter=f"tcp and dst port {SERV_PORT}", prn=packet_callback, store=0)