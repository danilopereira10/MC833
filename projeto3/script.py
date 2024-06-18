import nest_asyncio
nest_asyncio.apply()
from scapy.all import *

def get_icmp_packets(packets, srcip, destip):
    icmp_packets = [packet for packet in packets if ICMP in packet]
    packetsFromSource = [packet for packet in icmp_packets if packet[IP].src == srcip]
    packetsFromSourceToDestination = [packet for packet in packetsFromSource if packet[IP].dst == destip]

    return packetsFromSourceToDestination

def amountOfBytes(packets):
    totalBytes = 0
    for index in range(1, len(packets)):
        totalBytes += len(packets[index])
    return totalBytes


def print_info(packets, sourceIp, destinationIp):
    packets = get_icmp_packets(packets, sourceIp, destinationIp)
    print("src: ", sourceIp)
    print("dest: ", destinationIp)
    
    highestArrivalTime = packets[-1].time
    totalBytesSent = amountOfBytes(packets)
    print("Throughput:", (totalBytesSent) / (highestArrivalTime-packets[0].time), "bytes per second")
    averageIntervalDifference = (highestArrivalTime-packets[0].time) / (len(packets)-1)
    print("Intervalo médio entre os pacotes:", averageIntervalDifference, "segundos")
    print("Total de pacotes:", len(packets))
    print()


if __name__=='__main__':
    pcap_file = "c.pcap"
    packets = rdpcap(pcap_file)
    print_info(packets, sourceIp="10.0.0.1", destinationIp="10.0.0.3")
    print_info(packets, sourceIp="10.0.0.3", destinationIp="10.0.0.1")
    print_info(packets, sourceIp="10.0.0.2", destinationIp="10.0.0.4")
    print_info(packets, sourceIp="10.0.0.4", destinationIp="10.0.0.2")
