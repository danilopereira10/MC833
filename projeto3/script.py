import nest_asyncio
nest_asyncio.apply()
# import pandas as pd
from scapy.all import *
# import glob
# import numpy as np
import os
# from pprint import pprint

def get_icmp_packets(packets, srcip, destip):
    icmp_packets = [pkt for pkt in packets if ICMP in pkt]
    # for i in icmp_packets:
    #     print(i)
    # print(icmp_packets[0].show())
    srcpackets = [pkt for pkt in icmp_packets if pkt[IP].src == srcip]
    # for i in packets0:
        # print(i.sent)
        # print(i.time)
        # print(len(i))
        # print(i.sprintf("%IP.len%"))
    srcToDestPackets = [pkt for pkt in srcpackets if pkt[IP].dst == destip]

    return srcToDestPackets

def amountOfBytes(packets):
    t = 0
    for i in range(1, len(packets)):
        t += len(packets[i])
    return t

def min(packets):
    p = packets[0]
    for pkt in packets:
        if pkt.sent_time < p.sent_time:
            p = pkt
    return p.sent_time

def max(packets):
    p = packets[0]
    for pkt in packets:
        if pkt.time > p.time:
            p = pkt
    return p.time

def interval_difference(packets):
    t = 0
    for i in range(1, len(packets)):
        t += packets[i].time - packets[i-1].time
    return t / (len(packets)-1)


def print_info(packets, srcip, destip):
    packets = get_icmp_packets(packets, srcip, destip)
    print("src: ", srcip)
    print("dest: ", destip)
    
    # m = min(packets1)
    ma = packets[-1].time
    n = amountOfBytes(packets)
    print("Throughput:", (n) / (ma-packets[0].time), "bytes per second")
    averageIntervalDifference = (ma-packets[0].time) / (len(packets)-1)
    print("Intervalo médio entre os pacotes:", averageIntervalDifference, "segundos")
    print("Total de pacotes:", len(packets))
    print()
    
    


if __name__=='__main__':
    pcap_file = "cap.pcap"
    packets = rdpcap(pcap_file)
    print_info(packets, srcip="10.0.0.1", destip="10.0.0.3")
    print_info(packets, srcip="10.0.0.3", destip="10.0.0.1")
    print_info(packets, srcip="10.0.0.2", destip="10.0.0.4")
    print_info(packets, srcip="10.0.0.4", destip="10.0.0.2")
    

# ans = sr([IP(dst="8.8.8.8", ttl=(1, 8), options=IPOption_RR())/ICMP(seq=RandShort()), IP(dst="8.8.8.8", ttl=(1, 8), options=IPOption_Traceroute())/ICMP(seq=RandShort()), IP(dst="8.8.8.8", ttl=(1, 8))/ICMP(seq=RandShort())], verbose=False, timeout=3)[0]
# ans.make_table(lambda x, y: (", ".join(z.summary() for z in x[IP].options) or '-', x[IP].ttl, y.sprintf("%IP.src% %ICMP.type%")))



# <ICMP  type=time-exceeded code=ttl-zero-during-transit chksum=0x50d6 reserved=0 length=0 unused=None |<IPerror  version=4L ihl=5L tos=0x0 len=61 id=1 flags= frag=0L ttl=1 proto=udp chksum=0xf389 src=172.20.10.2 dst=8.8.8.8 options=[] |<UDPerror  sport=domain dport=domain len=41 chksum=0x593a |<DNS  id=0 qr=0L opcode=QUERY aa=0L tc=0L rd=1L ra=0L z=0L ad=0L cd=0L rcode=ok qdcount=1 ancount=0 nscount=0 arcount=0 qd=<DNSQR  qname='www.example.com.' qtype=A qclass=IN |> an=None ns=None ar=None |>>>>

# <Ether  dst=f4:ce:46:a9:e0:4b src=34:95:db:04:3c:29 type=IPv4 |<IP version=4L ihl=5L tos=0x0 len=61 id=1 flags= frag=0L ttl=5 proto=udp chksum=0xb6e3 src=192.168.46.20 dst=8.8.8.8 options=[] |<UDP sport=domain dport=domain len=41 chksum=0xb609 |<DNS  id=0 qr=0L opcode=QUERY aa=0L tc=0L rd=1L ra=0L z=0L ad=0L cd=0L rcode=ok qdcount=1 ancount=0 nscount=0 arcount=0 qd=<DNSQR  qname='www.example.com.' qtype=A qclass=IN |> an=None ns=None ar=None |>>>>


# <DNS  id=0 qr=1L opcode=QUERY aa=0L tc=0L rd=1L ra=1L z=0L ad=0L cd=0L rcode=ok qdcount=1 ancount=1 nscount=0 arcount=0 qd=<DNSQR  qname='www.example.com.' qtype=A qclass=IN |> an=<DNSRR  rrname='www.example.com.' type=A rclass=IN ttl=19681 rdata='93.184.216.34' |> ns=None ar=None |>

