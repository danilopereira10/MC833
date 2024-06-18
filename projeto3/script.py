import nest_asyncio
nest_asyncio.apply()
from scapy.all import *

#Método para obter os pacotes ICMP de um determinado host para outro
def get_icmp_packets(packets, sourceIp, destinationIp):
    icmp_packets = [packet for packet in packets if ICMP in packet]
    packetsFromSource = [packet for packet in icmp_packets if packet[IP].src == sourceIp]
    packetsFromSourceToDestination = [packet for packet in packetsFromSource if packet[IP].dst == destinationIp]

    return packetsFromSourceToDestination

#Método para calcular a quantidade de bytes presente numa lista de pacotes, desconsiderando o primeiro pacote
def amountOfBytes(packets):
    totalBytes = 0
    for index in range(1, len(packets)):
        totalBytes += len(packets[index])
    return totalBytes


#Método para imprimir as informações desejadas
def print_info(allPackets, sourceIp, destinationIp):
    sentPackets = get_icmp_packets(allPackets, sourceIp, destinationIp)
    print("src: ", sourceIp)
    print("dest: ", destinationIp)

    receivedPackets = get_icmp_packets(allPackets, sourceIp=destinationIp, destinationIp=sourceIp)
    
    highestArrivalTime = sentPackets[-1].time
    totalBytesSent = amountOfBytes(sentPackets)
    print("Throughput:", totalBytesSent / (highestArrivalTime-sentPackets[0].time), "bytes por segundo")
    averageIntervalDifference = (highestArrivalTime-receivedPackets[0].time) / (len(sentPackets)-1)
    print("Intervalo médio De chegada de pacotes:", averageIntervalDifference, "segundos")
    print("Total de pacotes enviados:", len(sentPackets))
    print()


if __name__=='__main__':
    pcap_file1 = "cap1.pcap"
    pcap_file2 = "cap2.pcap"
    packets1 = rdpcap(pcap_file1)
    packets2 = rdpcap(pcap_file2)
    print_info(packets1, sourceIp="10.0.0.1", destinationIp="10.0.0.3")
    print_info(packets2, sourceIp="10.0.0.2", destinationIp="10.0.0.4")
