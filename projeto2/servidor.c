/*
** server.c -- a stream socket server demo
*/
// #define _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include <sys/fcntl.h>





int max(int a, int b) {
	if (a > b) {
		return a;
	}
	return b;
}

int min(int a, int b) {
	if (a < b) {
		return a;
	}
	return b;
}




/*Método para filtragem (verificar se identificado bate com identificador,
tipo de música bate com tipo de música, etc...)*/
int startsWith(const char *a, const char *b) {
	if (strlen(a) >= strlen(b)) {
		if (strncmp(a,b, strlen(b)) == 0) {
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{


	struct addrinfo tcpHints, *res;
	int tcpSocket;
	char inputBuffer[1024];
	char bufferToBeSentToClient[4096];
	int bytesReceived;

    struct sockaddr_storage their_addr;

	memset(&tcpHints, 0, sizeof tcpHints);
	tcpHints.ai_family = AF_UNSPEC;
	tcpHints.ai_socktype = SOCK_STREAM;
	tcpHints.ai_flags = AI_PASSIVE;


	int rv;

	char* porttcp = argv[1];
	char* portudp = argv[2];
    if ((rv = getaddrinfo(NULL, argv[1], &tcpHints, &res)) != 0 ) {
        fprintf(stderr, "getaddrinfo tcp: %s \n", gai_strerror(rv));
    }

    struct addrinfo *p;
	//tentativa de criação e binding de socket
    for (p = res; p != NULL; p = p->ai_next) {
        if ((tcpSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("tcp socket \n");
            continue;
        }
		int yes = 1;
		if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			printf("erro no setsockopt \n");
		}
		if (bind(tcpSocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(tcpSocket);
			printf("erro no bind \n");
			continue;
		}
        break;
    }



	if (listen(tcpSocket, 10) == -1) {
		printf("erro no listen \n");
	}
    socklen_t addr_size;
    addr_size = sizeof their_addr;

	/* socket para UDP. Abaixo seguem todas as variáveis e criações
		correspondentes.
	*/
	int datagramSocket;
	struct addrinfo datagramHints, *dservinfo, *dp;
	int datagramRv;
	int datagramNumbytes;
	struct sockaddr_storage datagramClient_addr;
	// char dbuf[MAXBUFLEN];
	socklen_t datagramClientaddr_len;
	char ds[INET6_ADDRSTRLEN];

	memset(&datagramHints, 0, sizeof datagramHints);
	datagramHints.ai_family = AF_UNSPEC;
	datagramHints.ai_socktype = SOCK_DGRAM;
	datagramHints.ai_flags = AI_PASSIVE;

	if ((datagramRv = getaddrinfo(NULL, argv[2], &datagramHints, &dservinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(datagramRv));
		return 1;
	}


	//Tentativa de criação socket UDP
	for (dp = dservinfo; dp != NULL; dp=dp->ai_next) {
		if ((datagramSocket = socket(dp->ai_family, dp->ai_socktype, dp->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		int yes2 = 1;
		if (setsockopt(datagramSocket, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof yes2) == -1) {
			printf("erro no setsockopt 2\n");
		}

		if ((bind(datagramSocket, dp->ai_addr, dp->ai_addrlen)) == -1) {
			close(datagramSocket);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (dp == NULL) {
		fprintf(stderr, "listener: failed to bind socket \n");
		return 2;
	}
	freeaddrinfo(dservinfo);

	datagramClientaddr_len = sizeof datagramClient_addr;


	/* Tornar socket UDP não bloqueante.
		No manual do Beej.s diz que o select pode dizer que o socket
		está pronto para "recv" no Linux mesmo sem estar. Eles sugerem
		o método fcntl para evitar o bloqueio do socket.
	*/

	fcntl(tcpSocket, F_SETFL, O_NONBLOCK);
	fcntl(datagramSocket, F_SETFL, O_NONBLOCK);

	fd_set readfds;




	/* essas variáveis são utilizadas para recebimento de dados pela porta UDP.
	Elas são declaradas força do laço
	*/
	int i2 = 1, i5 = 0;
	while (1) {

		/* Preparação de variáveis para select
			O select atua entre receber conexões na porta TCP ou
			receber requisições pela porta UDP
		*/
		struct timeval tv;
		FD_ZERO(&readfds);

		FD_SET(tcpSocket, &readfds);
		FD_SET(datagramSocket, &readfds);
		int highestSocket = max(datagramSocket,tcpSocket) + 1;
		//Intervalo de 1 segundo.
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		datagramRv = select(highestSocket, &readfds, NULL, NULL, &tv);
		int totalBytesSent = 0;
		if (datagramRv == -1) {
			perror("select");
			i5 = 0;
			i2 = 1;
		} else if (datagramRv == 0) {
			i5 = 0;
			i2 = 1;
			//Nenhum dado ou pedido de conexão recebidos
			//printf("Timeout occurred! No data after 1 second. \n");
		} else {

			if (FD_ISSET(tcpSocket, &readfds)) {
				//Select apontou recebimento de pedido de conexão
				int new_fd;
				if ((new_fd = accept(tcpSocket, (struct sockaddr *) &their_addr, &addr_size)) == -1) {
					// Não foi recebido pedido de conexão
					continue;
				}

				printf("Recebida conexão \n");
				pid_t pid;

				if ((pid = fork()) == 0) {
					close(tcpSocket);


					while (1) {
						int totalBytesSent = 0;
						int bytesRemainingToBeSent = 1024;
						int counter = 0;

						// Conta a quantidade de caracteres '\0'
						int nullcharacterCounter = 0;

						// Verifica se já foram recebidos todos os dados necessários para
						// processar a requisição
						int receivedEnoughDataToProcessRequest = 0;

						// Identifica fechamento de conexão
						int connectionClosed = 0;
						while (1) {
							int totalBytesSent = 0;
							int bytesRemainingToBeSent = 1024;
							bytesReceived = recv(new_fd, inputBuffer+0, 1024, 0);
							printf("recv()'d %d bytes of data in buf\n", bytesReceived);
							if ((bytesReceived == -1)) {
								// Erro no recebimento de dados
							} else if (bytesReceived == 0) {
								connectionClosed = 1;
								break;
							} else {
								totalBytesSent += bytesReceived;
								bytesRemainingToBeSent -= bytesReceived;
								while (counter < totalBytesSent) {
									if ((inputBuffer[counter] == '\0')) {
										//A variável c conta a quantidade de '\0'
										/* Os '\0' separam os dados que foram enviados pelo
										cliente */
										nullcharacterCounter++;
									}
									counter++;
									// Apenas a operação '4' recebe dois dados.
									// buf[0] Sempre é a operação (varia de '1' a '8').
									if (nullcharacterCounter && inputBuffer[0] != '4') {
										receivedEnoughDataToProcessRequest = 1;
										break;
									} else if (nullcharacterCounter == 2) {
										receivedEnoughDataToProcessRequest = 1;
										break;
									}
								}
							}
							if (receivedEnoughDataToProcessRequest || connectionClosed) {
								// Recebidos todos os dados ou conexão finalizada
								break;
							}
						}
						if (connectionClosed) {
							// Conexão finalizada - o socket será fechado lá embaixo.
							break;
						}

						// Os dados foram recebidos e conexão não foi finalizada.
						// Abrindo arquivo da descrição das músicas.
						FILE *filePointer;
						filePointer = fopen("musicas", "r");;

						long fileSizeInBytes;
						char* line = NULL;
						ssize_t read;
						if (inputBuffer[0] == '7') {
							//Copia todo o conteúdo do arquivo e retorna para o cliente
							fseek(filePointer, 0, SEEK_END);
							fileSizeInBytes = ftell(filePointer);
							fseek(filePointer, 0, SEEK_SET);
							rewind(filePointer);
							fread(bufferToBeSentToClient, 1, fileSizeInBytes, filePointer);
							int totalBytesSent = 0;

							int bytesReceived;
							bufferToBeSentToClient[fileSizeInBytes] = '\0';
							int len = fileSizeInBytes+1;
							int bytesRemainingToBeSent = len;

							while (totalBytesSent < len) {
								if ((bytesReceived = send(new_fd, bufferToBeSentToClient+totalBytesSent, bytesRemainingToBeSent, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesReceived);
								totalBytesSent += bytesReceived;
								bytesRemainingToBeSent -= bytesReceived;
							}
						} else if (inputBuffer[0] == '6') {
							// Faz match da linha que possuir mesmo identificador e retorna linha para o cliente
							int identifier;
							char identifierString[1024];
							int i2 = 1;
							while (inputBuffer[i2] != '\0') {
								identifierString[i2-1] = inputBuffer[i2];
								i2++;
							}
							identifierString[i2-1] = inputBuffer[i2];
							identifier = atoi(identifierString);
							char identifierStringWithPrefix[1024];
							char buf2[1024];
							int i3 = 0, i4 = 0, previousI3 = 0;
							int lineLength = 0;
							snprintf(identifierStringWithPrefix, 1024, "Identificador Único: %d\n", identifier);
							int hasTheSameIdentifierReceivedInInput = 0, lineStartsWithUniqueIdentifierString = 0;
							while ((read = getline(&line, &lineLength, filePointer)) != -1) {
								printf(line);
								if (startsWith(line, "Identificador Único:")) {
									if (hasTheSameIdentifierReceivedInInput) {
										lineStartsWithUniqueIdentifierString = 1;
										previousI3 = i3;
										break;

									}
									i3 = previousI3;
									hasTheSameIdentifierReceivedInInput = startsWith(line, identifierStringWithPrefix);
								}

								i4 = 0;
								while (line[i4] != '\0') {
									buf2[i3] = line[i4];
									i3++;
									i4++;
								}
							}
							buf2[i3] = '\0';
							int i5 = 0;
							while (i5 < i3) {
								bufferToBeSentToClient[i5] = buf2[i5];
								i5++;
							}
							if (i5 == 0) {
								bufferToBeSentToClient[0] = '\0';
							}
							if (!lineStartsWithUniqueIdentifierString) {
								i3 = 0;
							}
							int bytesReceived;
							bufferToBeSentToClient[i3] = '\0';
							lineLength = i3+1;
							int bytesRemainingToBeSent = lineLength;

							while (totalBytesSent < lineLength) {
								if ((bytesReceived = send(new_fd, bufferToBeSentToClient+total, bytesRemainingToBeSent, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesReceived);
								total += bytesReceived;
								bytesRemainingToBeSent -= bytesReceived;
							}

						} else if (inputBuffer[0] == '5') {
							// Faz match das músicas que possuem o mesmo tipo e retorna para o cliente
							char tipo[1024];
							int i2 = 1;
							while (inputBuffer[i2] != '\0') {
								tipo[i2-1] = inputBuffer[i2];
								i2++;
							}
							tipo[i2-1] = inputBuffer[i2];

							char tipo2[1024];
							char buf2[1024];
							int i3 = 0, i4 = 0, pi3 = 0;
							snprintf(tipo2, 1024, "Tipo de música: %s\n", tipo);
							int len = 0;
							int c2 = 0, c3 = 0;
							while ((read = getline(&line, &len, filePointer)) != -1) {
								if (startsWith(line, "Tipo de música:")) {
									c2 = startsWith(line, tipo2);
									if (c2) {
										c3 = 1;
										pi3 = i3;
									}
									i3 = pi3;
								}

								if (startsWith(line, "Identificador Único")|| startsWith(line, "Título") || startsWith(line, "Intérprete")) {
									i4 = 0;
									while (line[i4] != '\0') {
										buf2[i3] = line[i4];
										i3++;
										i4++;
									}
								}
							}

							buf2[i3] = '\0';
							int i5 = 0;
							if (i3) {
								while (i5 < i3) {
									bufferToBeSentToClient[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufferToBeSentToClient[0] = '\0';
							}

							if (!c3) {
								i3 = 0;
							}
							int bytesReceived;
							bufferToBeSentToClient[i3] = '\0';
							len = i3+1;
							int bytesRemainingToBeSent = len;

							while (total < len) {
								if ((bytesReceived = send(new_fd, bufferToBeSentToClient+total, bytesRemainingToBeSent, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesReceived);
								total += bytesReceived;
								bytesRemainingToBeSent -= bytesReceived;
							}
						} else if (inputBuffer[0] == '3') {
							// Faz match das músicas que possuem o mesmo Ano de lançamento e retorna para o cliente
							char ano[1024];
							int i2 = 1;
							while (inputBuffer[i2] != '\0') {
								ano[i2-1] = inputBuffer[i2];
								i2++;
							}
							ano[i2-1] = inputBuffer[i2];

							char ano2[1024];
							char buf2[1024];
							int i3 = 0, i4 = 0, pi3 = 0;
							int anoint = atoi(ano);
							snprintf(ano2, 1024, "Ano de lançamento: %d\n", anoint);
							int lineLength = 0;
							int hasTheSameIdentifierReceivedInInput = 0, lineStartsWithUniqueIdentifierString = 0;
							while ((read = getline(&line, &lineLength, filePointer)) != -1) {
								if (startsWith(line, "Ano de lançamento:")) {
									hasTheSameIdentifierReceivedInInput = startsWith(line, ano2);
									if (hasTheSameIdentifierReceivedInInput) {
										lineStartsWithUniqueIdentifierString = 1;
										pi3 = i3;
									}
									i3 = pi3;

								}


								if (startsWith(line, "Identificador Único")|| startsWith(line, "Título") || startsWith(line, "Intérprete")) {
									i4 = 0;
									while (line[i4] != '\0') {
										buf2[i3] = line[i4];
										i3++;
										i4++;
									}
								}
							}
							buf2[i3] = '\0';
							int i5 = 0;
							if (i3) {
								while (i5 < i3) {
									bufferToBeSentToClient[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufferToBeSentToClient[0] = '\0';
							}
							if (!lineStartsWithUniqueIdentifierString) {
								i3 = 0;
							}
							int bytesReceived;
							bufferToBeSentToClient[i3] = '\0';
							lineLength = i3+1;
							int bytesRemainingToBeSent = lineLength;

							while (total < lineLength) {
								if ((bytesReceived = send(new_fd, bufferToBeSentToClient+total, bytesRemainingToBeSent, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesReceived);
								total += bytesReceived;
								bytesRemainingToBeSent -= bytesReceived;
							}
						} else if (inputBuffer[0] == '4') {
							// Faz match das músicas que possuem mesmo Ano de Lançamento e Idioma e retorna para o cliente
							char ano[1024];
							char idioma[1024];
							int i2 = 1;
							int i3 = 0, i4 = 0;
							while (inputBuffer[i2] != '\0') {
								idioma[i3] = inputBuffer[i2];
								i2++;
								i3++;
							}
							idioma[i3] = inputBuffer[i2];
							i2++;

							while (inputBuffer[i2] != '\0') {
								ano[i4] = inputBuffer[i2];
								i2++;
								i4++;
							}
							ano[i4] = inputBuffer[i2];

							char ano2[1024];
							char idioma2[1024];
							char buf2[1024];
							i3 = 0, i4 = 0;
							int pi3 = 0;
							int anoint = atoi(ano);
							snprintf(ano2, 1024, "Ano de lançamento: %d\n", anoint);
							int len = 0;
							snprintf(idioma2, 1024, "Idioma: %s\n", idioma);
							int c2 = 0, c3 = 0;
							while ((read = getline(&line, &len, filePointer)) != -1) {
								if (startsWith(line, "Idioma:")) {
									c2 = 0;
									if (startsWith(line, idioma2)) {
										c2++;
									}
								} else if (startsWith(line, "Ano de lançamento:")) {
									if (startsWith(line, ano2) && c2) {
										c2++;
									} else {
										c2 = 0;
									}

									if (c2 == 2) {
										c3 = 1;
										pi3 = i3;
									} else {
										i3 = pi3;
									}
								}


								if (startsWith(line, "Identificador Único")|| startsWith(line, "Título") || startsWith(line, "Intérprete")) {
									i4 = 0;
									while (line[i4] != '\0') {
										buf2[i3] = line[i4];
										i3++;
										i4++;
									}
								}
							}
							buf2[i3] = '\0';
							int i5 = 0;
							if (i3) {
								while (i5 < i3) {
									bufferToBeSentToClient[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufferToBeSentToClient[0] = '\0';
							}
							if (!c3) {
								i3 = 0;
							}
							int bytesReceived;
							bufferToBeSentToClient[i3] = '\0';
							len = i3+1;
							int bytesRemainingToBeSent = len;

							while (total < len) {
								if ((bytesReceived = send(new_fd, bufferToBeSentToClient+total, bytesRemainingToBeSent, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesReceived);
								totalBytesSent += bytesReceived;
								bytesRemainingToBeSent -= bytesReceived;
							}
						} else if (inputBuffer[0] == '2') {
							// Faz match da música que possui o mesmo identificador e reescreve o arquivo de músicas sem ela.
							int identifier;
							char identifierString[1024];
							int i2 = 1;
							while (inputBuffer[i2] != '\0') {
								identifierString[i2-1] = inputBuffer[i2];
								i2++;
							}
							identifierString[i2-1] = inputBuffer[i2];
							identifier = atoi(identifierString);
							char identifierStringWithPrefix[1024];
							char buf2[4096];
							int i3 = 0, i4 = 0, previousI3 = 0;
							snprintf(identifierStringWithPrefix, 1024, "Identificador Único: %d\n", identifier);
							int lineLength = 0;
							int hasTheSameIdentifierReceivedInInput = 0;
							while ((read = getline(&line, &lineLength, filePointer)) != -1) {

								if (startsWith(line, "Identificador Único:")) {
									if (hasTheSameIdentifierReceivedInInput) {
										i3 = previousI3;
									}
									previousI3 = i3;
									// i3 = pi3;
									hasTheSameIdentifierReceivedInInput = startsWith(line, identifierStringWithPrefix);
								}
								if (hasTheSameIdentifierReceivedInInput) {
									continue;
								}

								i4 = 0;
								while (line[i4] != '\0') {
									buf2[i3] = line[i4];
									i3++;
									i4++;
								}
							}
							buf2[i3] = '\0';
							int i5 = 0;
							if (i3) {
								while (i5 < i3) {
									bufferToBeSentToClient[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufferToBeSentToClient[0] = '\0';
							}
							fclose(filePointer);
							filePointer = fopen("musicas", "w");
							fwrite(bufferToBeSentToClient, 1, i3, filePointer);
						}  else if (inputBuffer[0] == '1') {
							// Copia todo o arquivo de músicas e adiciona a nova música se já não houver outra música com o mesmo identificador
							// con = identificador + informações
							char con[1024];
							int id;
							// idc = "identificador" + o número
							char idc[1024];
							int i2 = 1;
							while (inputBuffer[i2] != '\bytesReceived') {
								idc[i2-1] = inputBuffer[i2];
								con[i2-1] = inputBuffer[i2];
								i2++;
							}
							while (inputBuffer[i2] != '\0') {
								con[i2-1] = inputBuffer[i2];
								i2++;
							}
							idc[i2-1] = inputBuffer[i2];
							con[i2-1] = inputBuffer[i2];
							id = atoi(idc);
							int len = 0;
							char idc2[1024];
							char buf2[4096];
							int i3 = 0, i4 = 0, pi3 = 0;
							// Junta string "identificador" com o número
							snprintf(idc2, 1024, "Identificador Único: %d\n", id);
							int c2 = 0;
							while ((read = getline(&line, &len, filePointer)) != -1) {

								if (startsWith(line, "Identificador Único:")) {
									c2 = startsWith(line, idc2);
									if (c2) {
										break;
									}
								}

								i4 = 0;
								while (line[i4] != '\0') {
									buf2[i3] = line[i4];
									i3++;
									i4++;
								}
							}
							if (c2) {
								// Já encontrada música com mesmo identificador
							} else {
								int i5 = 0;
								while (con[i5] != '\0') {
									buf2[i3] = con[i5];
									i3++;
									i5++;
								}
								buf2[i3] = '\0';
								i5 = 0;
								if (i3) {
									while (i5 < i3) {
										bufferToBeSentToClient[i5] = buf2[i5];
										i5++;
									}
								} else {
									bufferToBeSentToClient[0] = '\0';
								}
								fclose(filePointer);
								filePointer = fopen("musicas", "w");
								fwrite(bufferToBeSentToClient, 1, i3, filePointer);
							}




						}
						fclose(filePointer);
					}


					close(new_fd);
					exit(0);
				}
				 close(new_fd);

			}
			if (FD_ISSET(datagramSocket, &readfds)) {


				bytesReceived = recvfrom(datagramSocket, inputBuffer, 1024, 0, ((struct sockaddr *)&datagramClient_addr), &datagramClientaddr_len);
				if (bytesReceived == -1) {
					/* Potencialmente nada para receber
						- Select acionou quando não deveria
					*/
					continue;
				}
				total += bytesReceived;
				// bytesleft -= n;
				i5 = 0;
				i2 = 1;
				char idc[1024];
				int id;
				while (i5 < total) {
					if (inputBuffer[i5] == '\0') {
						while (inputBuffer[i2] != '\0') {
							idc[i2-1] = inputBuffer[i2];
							i2++;
						}
						idc[i2-1] = inputBuffer[i2];
						id = atoi(idc);


						char buf2[1024];

						FILE *fptr;
						char filename[13];
						filename[0] = '\0';
						snprintf(filename, 13, "%d.mp3", id);
						fptr = fopen(filename, "rb");
						if (fptr != NULL) {
							// fgets(bufout, 4096, fptr);
							long size;
							char* line = NULL;
							int len = 0;
							ssize_t read;

							fseek(fptr, 0, SEEK_END);
							size = ftell(fptr);
							int rate = 1000;
							int am = size / 1000;

							char* bufout2 = malloc((size) * sizeof(char));
							fseek(fptr, 0, SEEK_SET);
							rewind(fptr);
							fread(bufout2, 1, size, fptr);
							// size = size + 6*(am+1);
							int total = 0;
							int bytesleft = size;

							int j = 0;

							int tt = 0;
							while (j <= am) {
								if (j == 749) {
									printf("opa\n");
								}
								int s2 = 7;
								char str[s2];
								sprintf(str, "%06d", j);
								char str2[6 + min(bytesleft, rate)];
								memcpy(str2, str, 7);
								memcpy(str2+6, bufout2+total, min(bytesleft, rate));
								int total2 = 0, bytesleft2 = 6 + min(bytesleft, rate);
								int len2 = bytesleft2;
								// Envia pacotes de tamanho máximo bytelefts2 = 1006 para evitar desalinhamento e consequentes descartes
								while (total2 < len2) {

									if ((bytesReceived = sendto(datagramSocket, str2, bytesleft2, 0, (struct sockaddr*)&datagramClient_addr, datagramClientaddr_len)) == -1 ) {
										// printf("Erro no envio n=-1 op8 \n");
										//skip packet
										total2 = len2;
										bytesleft2 = 0;

										continue;
									}

									tt += bytesReceived;
									if (bytesReceived != 1006) {
										// printf("opa\n");
									}
									printf("Enviados %d bytes \n", bytesReceived);
									total2 += bytesReceived;
									bytesleft2 -= bytesReceived;
								}
								printf("%d\n", j);
								printf("%d \n", bytesleft);
								j++;
								total += total2 - 6;
								bytesleft -= total2 - 6;

							}
							free(bufout2);
							printf("tt: %d \n", tt);

						}
						fclose(fptr);
						total = 0;
						inputBuffer[0] = '\0';
						inputBuffer[1] = '\0';
						i5 = 0;
						i2 = 1;
						break;
					}
					i5++;
				}
			}
		}

	}
	return 0;
}

