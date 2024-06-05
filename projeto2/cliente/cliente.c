/*
** server.c -- a stream socket server demo
*/
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>

#define PORT "3490"  // the port users will be connecting to
#define PORT2 "3491"

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

#define MAXBUFLEN 100
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int max(int a, int b) {
	if (a > b) {
		return a;
	}
	return b;
}

#define MYPORT "3490"
int main(int argc, char* argv[]) {
	char* serverip = argv[1];
	char* porttcp = argv[2];
	char* portudp = argv[3];

	struct addrinfo tcpHints, *tcpRes;
    int tcpSocket;

    memset(&tcpHints, 0, sizeof tcpHints);
    tcpHints.ai_family = AF_UNSPEC;     
    tcpHints.ai_socktype = SOCK_STREAM; 


 
    int rv;

    if ((rv = getaddrinfo(serverip, porttcp, &tcpHints, &tcpRes)) != 0) {
        fprintf(stderr, "getaddrinfo tcp: %s\n", gai_strerror(rv));
    }

    struct addrinfo *tcpP;
	//Tentativa de criação de socket TCP
    for (tcpP = tcpRes; tcpP != NULL; tcpP = tcpP->ai_next) {
        if ((tcpSocket = socket(tcpP->ai_family, tcpP->ai_socktype, tcpP->ai_protocol)) == -1) {
            perror("tcp socket \n");
            continue;
        }
        break;
    }


	int udpSocket;
	struct addrinfo udpHints, *udpServinfo, *udpP;
	int udpRv;
	int udpNumbytes;

	memset(&udpHints, 0, sizeof udpHints);
	udpHints.ai_family = AF_UNSPEC;
	udpHints.ai_socktype = SOCK_DGRAM;

	if ((udpRv = getaddrinfo(serverip, portudp, &udpHints, &udpServinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udpRv));
		return 1;
	}

	// Tentativa de criação de socket UDP
	for (udpP = udpServinfo; udpP != NULL; udpP = udpP->ai_next) {
		if ((udpSocket = socket(udpP->ai_family, udpP->ai_socktype, udpP->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}

	if (udpP == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	connect(tcpSocket, tcpP->ai_addr, tcpP->ai_addrlen);
	
	struct sockaddr_storage udp_their_addr;
	socklen_t uddp_addr_len;
	while (1) {
		int operation = 0;
		while (operation < 1 || operation > 9) {
			printf("Selecione a operação: \n");
			printf("1 - Cadastrar uma nova música utilizando um identificador \n");
			printf("2 - Remover uma música a partir de seu identificador \n");
			printf("3 - Listar todas as músicas (identificador, título e intérprete) lançadas em um determinado ano \n");
			printf("4 - Listar todas as músicas (identificador, título e intérprete) em um dado idioma lançadas em um dado ano \n");
			printf("5 - Listar todas as músicas de um certo tipo \n");
			printf("6 - Listar todas as informações de uma música dado o seu identificador \n");
			printf("7 - Listar todas as informações de todas as músicas \n");
			printf("8 - fazer um download de uma música a partir de seu identificador \n");
			printf("9 - Sair\n");
			scanf("%d", &operation);
		}
		if (operation == 9) {
			break;
		}
		// int op = atoi(argv[1]); 
		int i = 0;
		char outputBuffer[1024];
		outputBuffer[0] = '0' + operation;
		char filename[14];
		if ((operation!= 7)) {
			char content[1024];
			if ((operation == 2) || (operation == 6) || (operation == 8)) {
				printf("Digite o identificador: \n");
			} else if (operation == 3) {
				printf("Digite o ano: \n");
			} else if (operation == 4) {
				printf("Digite o idioma: \n");
			} else if (operation == 1) {
				printf("Coloque todas as informações da música (e digite Tab e aperte Enter no final): \n");
			} else {
				printf("Digite o tipo: \n");
			}
			
			if (operation != 1) {
				scanf("%s", content);
			} else {
				// Lê diversas linhas esperando um Tab. Então para.
				scanf("%[^\t]",content);
			}
			if (operation == 1) {
				while (content[i+1] != '\0') {
					outputBuffer[i+1] = content[i+1];
					i++;
				}
			} else {
				while (content[i] != '\0') {
					outputBuffer[i+1] = content[i];
					i++;
				}
			}
			if (operation == 8) {
				int j = 0;
				while (content[j] != '\0') {
					filename[j] = content[j];
					j++;
				}
				filename[j] = '.';
				filename[j+1] = 'm';
				filename[j+2] = 'p';
				filename[j+3] = '3';
				filename[j+4] = '\0';
			}

			char* content2 = NULL;
			
			if (operation == 4) {
				outputBuffer[i+1] = '\0';
				i++;
				printf("Digite o ano: \n");
				scanf("%ms", &content2);


				int j = 0;
				while (content2[j] != '\0') {
					outputBuffer[i+1] = content2[j];
					i++;
					j++;
				}
			}
			free(content2);
			
			

			i++;
			outputBuffer[i] = '\0';
			int totalBytesSent = 0;
			int bytesleft = i + 1, totalBytesToBeSent = i + 1;
			int bytesSent;
			if (operation != 8) {
				while (totalBytesSent < totalBytesToBeSent) {
					bytesSent  = send(tcpSocket, outputBuffer+totalBytesSent, bytesleft, 0);
					if (bytesSent == -1) {
						// printf("erro: n=-1 no op entre 1 e 6\n"); 
						break; 
					}
					totalBytesSent += bytesSent;
					bytesleft -= bytesSent;
				}
				char response[4096];
				response[0] = '\0';
				totalBytesSent = 0;
				bytesleft = 4096;
				int j = 0;
				int endOfResponseOfThisSpecificRequest = 0;
				if ((operation != 2) && (operation != 1)) {
					while (totalBytesSent < 4096) {
						bytesSent = recv(tcpSocket, response+totalBytesSent, bytesleft, 0);
						if (bytesSent == 0) {
							break;
						}
						totalBytesSent += bytesSent;
						bytesleft -= bytesSent;
						while (j < totalBytesSent) {
							if (response[j] == '\0') {
								endOfResponseOfThisSpecificRequest = 1;
								break;
							}
							j++;
						}
						if (endOfResponseOfThisSpecificRequest) {
							break;
						}
					}
					printf(response);
				}
			} else {
				while (totalBytesSent < totalBytesToBeSent) {
					bytesSent = sendto(udpSocket, outputBuffer+totalBytesSent, bytesleft, 0, udpP->ai_addr, udpP->ai_addrlen);
					if (bytesSent == -1) {
						// printf("erro n=-1 no op 8");
						break;
					}
					totalBytesSent += bytesSent;
					bytesleft -= bytesSent;
				}
				fd_set readfds;
				int total2 = 0;
				char* music = malloc(1024*1024*10 * sizeof(char));
				for (int i = 0; i< 1024*1024*10; i++) {
					music[i] = '\0';
				}
				bytesleft = 1024*1024*10;
				music[bytesleft-1] = '\0';
				int rate = 1000;
				char buft[rate+6];
				int amountOfBytes = 0;
				int highestSequenceControl2 = 0;
				int totalBytesReceived = 0;
				while (1) {
					struct timeval tv;
					FD_ZERO(&readfds);
					FD_SET(udpSocket, &readfds);
					int n2 = udpSocket+1;
					tv.tv_sec = 2;
					tv.tv_usec = 0;
					udpRv = select(n2, &readfds, NULL, NULL, &tv);
					if (udpRv == -1) {
						perror("select");	
						music[total2] = '\0';
						break;
					} else if (udpRv == 0) {
						music[total2] = '\0';
						break;
						//printf("Timeout occurred! No data after 1 second. \n");
					} else {
						int bytesReceived = recvfrom(udpSocket, buft, rate+6, 0, (struct sockaddr *)&udp_their_addr, &uddp_addr_len);
						
						// A variável t representa número de sequẽncia inválido.
						int invalidSequenceControlNumber = 0;
						for (int i = 0; (i < 6) && (i < bytesReceived); i++) {
							if ((buft[i] >'9') || (buft[i] < '0')) {
								invalidSequenceControlNumber = 1;
								break;
							}
						}
						// Os 6 primeiros bytes são apenas números de sequência. 
						// Se só tiver recebido números de sequẽncia ou houver menos número do que o necessário
						// para identificar qual é o número de sequência, então pula
						if ((bytesReceived <= 6) || invalidSequenceControlNumber) {
							printf("opa\n");
							continue;
						}
						totalBytesReceived += bytesReceived;
						char sequenceControl[7];
						memcpy(sequenceControl, buft, 6);
						sequenceControl[6] = '\0';
						// Converte de string para inteiro.
						int sequenceControlAsInteger = (int) strtol(sequenceControl, (char**) NULL, 10);
						int firstContentIndex = 6;
						int contentIndex = firstContentIndex;
						highestSequenceControl2 = max(highestSequenceControl2, sequenceControlAsInteger);
						

						while (contentIndex < bytesReceived) {
							if (sequenceControlAsInteger * rate + contentIndex < 1024*1024*10-1) {
								// Coloca os dados recebidos no número de sequẽncia correspondente.
								music[sequenceControlAsInteger * rate + contentIndex-firstContentIndex] = buft[contentIndex];
							}
							// A variável é utilizada como uma forma rudimentar de auferir o tamanho real da música,
							// ao invés de utilizar o tamanho 1024*1024*10 do tamanho do vetor bufin.
							amountOfBytes = max(amountOfBytes, sequenceControlAsInteger*rate+contentIndex-firstContentIndex);
							contentIndex++;
						}
					}

				}
				printf("%d\n", totalBytesReceived);
				FILE* filePointer = fopen(filename, "wb");
				amountOfBytes++;
				fwrite(music, 1, amountOfBytes, filePointer);
				fclose(filePointer);
				free(music);
			}
			
		} else {
			// Operação de listagem de todas as informações de todas as músicas. (Operação 7)
			// É enviada somente a operação.
			outputBuffer[0] = '7';
			outputBuffer[1] = '\0';
			int totalBytesSent = 0;
			int bytesleft = 2;
			int bytesToBeSent = 2;
			int bytesSent;
			while (totalBytesSent < bytesToBeSent) {
				bytesSent = send(tcpSocket, outputBuffer+totalBytesSent, bytesleft, 0);
				if (bytesSent == -1) {
					printf("erro n=-1 no envio op 7 \n");
					break;
				}
                printf("enviados %d bytess \n", bytesSent);
				totalBytesSent += bytesSent;
				bytesleft -= bytesSent;
			}
            char response[4096];
			response[0] = '\0';
            totalBytesSent = 0;
            bytesleft = 4096;
            int j = 0;
			int thisIsEnoughForTheResponseOfThisRequest = 0;
            while (totalBytesSent < 4096) {
                bytesSent = recv(tcpSocket, response+totalBytesSent, bytesleft, 0);
                totalBytesSent += bytesSent;
                bytesleft -= bytesSent;
                while (j < totalBytesSent) {
                    if (response[j] == '\0') {
						thisIsEnoughForTheResponseOfThisRequest = 1;
                        break;
                    }
                    j++;
                 }
				 if (thisIsEnoughForTheResponseOfThisRequest) {
					break;
				 }
            }
			printf(response);
            
		}
	}

	
	
	

	return 0;
}

