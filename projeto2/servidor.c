/*
** server.c -- a stream socket server demo
*/

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
	

	struct addrinfo tcpHints, *tcpRes;
	int tcpSockfd;
	char inputBuffer[1024];
	char outputBuffer[4096];
	int bytesReceivedOrSent;
    
    struct sockaddr_storage their_addr;

	memset(&tcpHints, 0, sizeof tcpHints);
	tcpHints.ai_family = AF_UNSPEC;  
	tcpHints.ai_socktype = SOCK_STREAM;
	tcpHints.ai_flags = AI_PASSIVE;


	int tcpRv;
    
	char* porttcp = argv[1];
	char* portudp = argv[2];
    if ((tcpRv = getaddrinfo(NULL, argv[1], &tcpHints, &tcpRes)) != 0 ) {
        fprintf(stderr, "getaddrinfo tcp: %s \n", gai_strerror(tcpRv));
    }

    struct addrinfo *tcpP;
	//tentativa de criação e binding de socket
    for (tcpP = tcpRes; tcpP != NULL; tcpP = tcpP->ai_next) {
        if ((tcpSockfd = socket(tcpP->ai_family, tcpP->ai_socktype, tcpP->ai_protocol)) == -1) {
            perror("tcp socket \n");
            continue;
        }
		int yes = 1;
		if (setsockopt(tcpSockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			printf("erro no setsockopt \n");
		}
		if (bind(tcpSockfd, tcpP->ai_addr, tcpP->ai_addrlen) == -1) {
			close(tcpSockfd);
			printf("erro no bind \n");
			continue;
		}
        break;
    }
    
	
	
	if (listen(tcpSockfd, 10) == -1) {
		printf("erro no listen \n");
	}
    socklen_t addr_size;
    addr_size = sizeof their_addr;

	/* socket para UDP. Abaixo seguem todas as variáveis e criações 
		correspondentes.
	*/
	int udpSocket;
	struct addrinfo udpHints, *udpServinfo, *udpP;
	int udpRv;
	int udpNumbytes;
	struct sockaddr_storage udpClient_addr;
	// char dbuf[MAXBUFLEN];
	socklen_t udpClientaddr_len;
	char ds[INET6_ADDRSTRLEN];

	memset(&udpHints, 0, sizeof udpHints);
	udpHints.ai_family = AF_UNSPEC;
	udpHints.ai_socktype = SOCK_DGRAM;
	udpHints.ai_flags = AI_PASSIVE;

	if ((udpRv = getaddrinfo(NULL, argv[2], &udpHints, &udpServinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udpRv));
		return 1;
	}

	
	//Tentativa de criação socket UDP
	for (udpP = udpServinfo; udpP != NULL; udpP=udpP->ai_next) {
		if ((udpSocket = socket(udpP->ai_family, udpP->ai_socktype, udpP->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		int yes = 1;
		if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			printf("erro no setsockopt 2\n");
		}

		if ((bind(udpSocket, udpP->ai_addr, udpP->ai_addrlen)) == -1) {
			close(udpSocket);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (udpP == NULL) {
		fprintf(stderr, "listener: failed to bind socket \n");
		return 2;
	}
	freeaddrinfo(udpServinfo);
	
	udpClientaddr_len = sizeof udpClient_addr;

	
	/* Tornar socket UDP não bloqueante.
		No manual do Beej.s diz que o select pode dizer que o socket
		está pronto para "recv" no Linux mesmo sem estar. Eles sugerem
		o método fcntl para evitar o bloqueio do socket.
	*/

	fcntl(tcpSockfd, F_SETFL, O_NONBLOCK);
	fcntl(udpSocket, F_SETFL, O_NONBLOCK);

	fd_set readfds;

	
	

	/* essas variáveis são utilizadas para recebimento de dados pela porta UDP.
	Elas são declaradas força do laço
	*/
	int counter = 1, counter2 = 0;
	while (1) {
		
		/* Preparação de variáveis para select
			O select atua entre receber conexões na porta TCP ou
			receber requisições pela porta UDP
		*/
		struct timeval tv;
		FD_ZERO(&readfds);

		FD_SET(tcpSockfd, &readfds);
		FD_SET(udpSocket, &readfds);
		int highestSocket = max(udpSocket,tcpSockfd) + 1;
		//Intervalo de 1 segundo.
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		udpRv = select(highestSocket, &readfds, NULL, NULL, &tv);
		int totalBytesReceivedOrSent = 0;
		if (udpRv == -1) {
			perror("select");
			counter2 = 0;
			counter = 1;
		} else if (udpRv == 0) {
			counter2 = 0;
			counter = 1;
			//Nenhum dado ou pedido de conexão recebidos
			//printf("Timeout occurred! No data after 1 second. \n");
		} else {
			
			if (FD_ISSET(tcpSockfd, &readfds)) {
				//Select apontou recebimento de pedido de conexão
				int new_fd;
				if ((new_fd = accept(tcpSockfd, (struct sockaddr *) &their_addr, &addr_size)) == -1) {
					// Não foi recebido pedido de conexão
					continue;
				}
				
				printf("Recebida conexão \n");
				pid_t pid;
				
				if ((pid = fork()) == 0) {
					close(tcpSockfd);
					
					
					while (1) {
						int totalBytesSent = 0;
						int bytesLeftToBeSent = 1024;
						int counter3 = 0;

						// Conta a quantidade de caracteres '\0'
						int nullCharacterCount = 0;

						// Verifica se já foram recebidos todos os dados necessários para
						// processar a requisição
						int hasEnoughDataToProcessRequest = 0;

						// Identifica fechamento de conexão
						int connectionClosed = 0;
						while (1) {
							int totalBytesToBeSent = 0;
							int bytesLeftToBeSent = 1024;
							bytesReceivedOrSent = recv(new_fd, inputBuffer+0, 1024, 0);
							printf("recv()'d %d bytes of data in buf\n", bytesReceivedOrSent);
							if ((bytesReceivedOrSent == -1)) { 
								// Erro no recebimento de dados
							} else if (bytesReceivedOrSent == 0) {
								connectionClosed = 1;
								break;
							} else {
								totalBytesToBeSent += bytesReceivedOrSent;
								bytesLeftToBeSent -= bytesReceivedOrSent;
								while (counter3 < totalBytesToBeSent) {
									if ((inputBuffer[counter3] == '\0')) {
										//A variável c conta a quantidade de '\0'
										/* Os '\0' separam os dados que foram enviados pelo
										cliente */
										nullCharacterCount++;
									}
									counter3++;
									// Apenas a operação '4' recebe dois dados.
									// buf[0] Sempre é a operação (varia de '1' a '8').
									if (nullCharacterCount && inputBuffer[0] != '4') {
										hasEnoughDataToProcessRequest = 1;
										break;
									} else if (nullCharacterCount == 2) {
										hasEnoughDataToProcessRequest = 1;
										break;
									}
								}
							}
							if (hasEnoughDataToProcessRequest || connectionClosed) {
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
						
						long fileSize;
						char* line = NULL;
						ssize_t readError;
						if (inputBuffer[0] == '7') {
							//Copia todo o conteúdo do arquivo e retorna para o cliente
							fseek(filePointer, 0, SEEK_END);
							fileSize = ftell(filePointer);
							fseek(filePointer, 0, SEEK_SET);
							rewind(filePointer);
							fread(outputBuffer, 1, fileSize, filePointer);
							int totalBytesSent = 0;
							
							int bytesSent;
							outputBuffer[fileSize] = '\0';
							int totalBytesToBeSent = fileSize+1;
							int bytesleft = totalBytesToBeSent;
							
							while (totalBytesSent < totalBytesToBeSent) {
								if ((bytesSent = send(new_fd, outputBuffer+totalBytesSent, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesSent);
								totalBytesSent += bytesSent;
								bytesleft -= bytesSent;
							}
						} else if (inputBuffer[0] == '6') {
							// Faz match da linha que possuir mesmo identificador e retorna linha para o cliente
							int identifier;
							char identifierString[1024];
							int counter4 = 1;
							while (inputBuffer[counter4] != '\0') {
								identifierString[counter4-1] = inputBuffer[counter4];
								counter4++;
							}
							identifierString[counter4-1] = inputBuffer[counter4];
							identifier = atoi(identifierString);
							char identifierStringWithPrefix[1024];
							char buf2[1024];
							int counter5 = 0, counter6 = 0, lineLength = 0;
							int amountOfBytesToBeSent = 0;
							snprintf(identifierStringWithPrefix, 1024, "Identificador Único: %d\n", identifier);
							int identifierMatchesSearchedOne = 0, foundAtLeastOneMusicThatMatches = 0;
							while ((readError = getline(&line, &lineLength, filePointer)) != -1) {
								// printf(line);
								if (startsWith(line, "Identificador Único:")) {
									if (identifierMatchesSearchedOne) {
										foundAtLeastOneMusicThatMatches = 1;
										amountOfBytesToBeSent = counter5;
										break;
										
									}
									counter5 = amountOfBytesToBeSent;
									identifierMatchesSearchedOne = startsWith(line, identifierStringWithPrefix);
								}
								
								counter6 = 0;
								while (line[counter6] != '\0') {
									buf2[counter5] = line[counter6];
									counter5++;
									counter6++;
								}	
							}
							buf2[counter5] = '\0'; 
							int counter7 = 0;
							while (counter7 < counter5) {
								outputBuffer[counter7] = buf2[counter7];
								counter7++;
							}
							if (counter7 == 0) {
								outputBuffer[0] = '\0';
							}
							if (!foundAtLeastOneMusicThatMatches) {
								counter5 = 0;
							}
							int bytesSent;
							outputBuffer[counter5] = '\0';
							amountOfBytesToBeSent = counter5+1;
							int bytesleft = amountOfBytesToBeSent;
							
							while (totalBytesSent < amountOfBytesToBeSent) {
								if ((bytesSent = send(new_fd, outputBuffer+totalBytesSent, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesSent);
								totalBytesSent += bytesSent;
								bytesleft -= bytesSent;
							}

						} else if (inputBuffer[0] == '5') {
							// Faz match das músicas que possuem o mesmo tipo e retorna para o cliente
							char musicType[1024];
							int index = 1;
							while (inputBuffer[index] != '\0') {
								musicType[index-1] = inputBuffer[index];
								index++;
							}
							musicType[index-1] = inputBuffer[index];
							
							char musicTypeWithPrefix[1024];
							char buf2[1024];
							int counter = 0, counter2 = 0, countedAmountOfBytesToBeSent = 0;
							snprintf(musicTypeWithPrefix, 1024, "Tipo de música: %s\n", musicType);
							int bytesToBeSent = 0;
							int matchedMusicType = 0, foundAtLeastOneMusicThatMatches = 0;
							while ((readError = getline(&line, &bytesToBeSent, filePointer)) != -1) {
								if (startsWith(line, "Tipo de música:")) {
									matchedMusicType = startsWith(line, musicTypeWithPrefix);
									if (matchedMusicType) {
										foundAtLeastOneMusicThatMatches = 1;
										countedAmountOfBytesToBeSent = counter;
									}
									counter = countedAmountOfBytesToBeSent;
								}
								
								if (startsWith(line, "Identificador Único")|| startsWith(line, "Título") || startsWith(line, "Intérprete")) {
									counter2 = 0;
									while (line[counter2] != '\0') {
										buf2[counter] = line[counter2];
										counter++;
										counter2++;
									}
								}	
							}
							
							buf2[counter] = '\0'; 
							int counter4 = 0;
							if (counter) {
								while (counter4 < counter) {
									outputBuffer[counter4] = buf2[counter4];
									counter4++;
								}
							} else {
								outputBuffer[0] = '\0';
							}

							if (!foundAtLeastOneMusicThatMatches) {
								counter = 0;
							}
							int bytesSent;
							outputBuffer[counter] = '\0';
							bytesToBeSent = counter+1;
							int bytesleft = bytesToBeSent;
							
							while (totalBytesSent < bytesToBeSent) {
								if ((bytesSent = send(new_fd, outputBuffer+totalBytesSent, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesSent);
								totalBytesSent += bytesSent;
								bytesleft -= bytesSent;
							}
						} else if (inputBuffer[0] == '3') {
							// Faz match das músicas que possuem o mesmo Ano de lançamento e retorna para o cliente
							char year[1024];
							int index = 1;
							while (inputBuffer[index] != '\0') {
								year[index-1] = inputBuffer[index];
								index++;
							}
							year[index-1] = inputBuffer[index];
							
							char yearWithPrefix[1024];
							char buf2[1024];
							int counter = 0, counter2 = 0, bytesToBeThoughtAbout = 0;
							int yearAsInteger = atoi(year);
							snprintf(yearWithPrefix, 1024, "Ano de lançamento: %d\n", yearAsInteger);
							int variableForCounting = 0;
							int matchesYear = 0, atLeastOneMatchFound = 0;
							while ((readError = getline(&line, &variableForCounting, filePointer)) != -1) {
								if (startsWith(line, "Ano de lançamento:")) {
									matchesYear = startsWith(line, yearWithPrefix);
									if (matchesYear) {
										atLeastOneMatchFound = 1;
										bytesToBeThoughtAbout = counter;
									}
									counter = bytesToBeThoughtAbout;
									
								}
								

								if (startsWith(line, "Identificador Único")|| startsWith(line, "Título") || startsWith(line, "Intérprete")) {
									counter2 = 0;
									while (line[counter2] != '\0') {
										buf2[counter] = line[counter2];
										counter++;
										counter2++;
									}
								}	
							}
							buf2[counter] = '\0'; 
							int counter4 = 0;
							if (counter) {
								while (counter4 < counter) {
									outputBuffer[counter4] = buf2[counter4];
									counter4++;
								}
							} else {
								outputBuffer[0] = '\0';
							}
							if (!atLeastOneMatchFound) {
								counter = 0;
							}
							int bytesSent;
							outputBuffer[counter] = '\0';
							variableForCounting = counter+1;
							int bytesleft = variableForCounting;
							
							while (totalBytesSent < variableForCounting) {
								if ((bytesSent = send(new_fd, outputBuffer+totalBytesSent, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesSent);
								totalBytesSent += bytesSent;
								bytesleft -= bytesSent;
							}
						} else if (inputBuffer[0] == '4') {
							// Faz match das músicas que possuem mesmo Ano de Lançamento e Idioma e retorna para o cliente
							char year[1024];
							char language[1024];
							int counter = 1;
							int counter2 = 0, counter3 = 0;
							while (inputBuffer[counter] != '\0') {
								language[counter2] = inputBuffer[counter];
								counter++;
								counter2++;
							}
							language[counter2] = inputBuffer[counter];
							counter++;

							while (inputBuffer[counter] != '\0') {
								year[counter3] = inputBuffer[counter];
								counter++;
								counter3++;
							}
							year[counter3] = inputBuffer[counter];
							
							char yearWithPrefix[1024];
							char languageWithPrefix[1024];
							char buf2[1024];
							counter2 = 0, counter3 = 0;
							int whereToResetCounter = 0;
							int yearAsInteger = atoi(year);
							snprintf(yearWithPrefix, 1024, "Ano de lançamento: %d\n", yearAsInteger);
							int variableForCounting = 0;
							snprintf(languageWithPrefix, 1024, "Idioma: %s\n", language);
							int matchesYearAndLanguage = 0, atLeastOneDoubleMatchFound = 0;
							while ((readError = getline(&line, &variableForCounting, filePointer)) != -1) {
								if (startsWith(line, "Idioma:")) {
									matchesYearAndLanguage = 0;
									if (startsWith(line, languageWithPrefix)) {
										matchesYearAndLanguage++;
									}
								} else if (startsWith(line, "Ano de lançamento:")) {	
									if (startsWith(line, yearWithPrefix) && matchesYearAndLanguage) {
										matchesYearAndLanguage++;
									} else {
										matchesYearAndLanguage = 0;
									}

									if (matchesYearAndLanguage == 2) {
										atLeastOneDoubleMatchFound = 1;
										whereToResetCounter = counter2;
									} else {
										counter2 = whereToResetCounter;
									}
								}
								

								if (startsWith(line, "Identificador Único")|| startsWith(line, "Título") || startsWith(line, "Intérprete")) {
									counter3 = 0;
									while (line[counter3] != '\0') {
										buf2[counter2] = line[counter3];
										counter2++;
										counter3++;
									}
								}	
							}
							buf2[counter2] = '\0'; 
							int counter5 = 0;
							if (counter2) {
								while (counter5 < counter2) {
									outputBuffer[counter5] = buf2[counter5];
									counter5++;
								}
							} else {
								outputBuffer[0] = '\0';
							}
							if (!atLeastOneDoubleMatchFound) {
								counter2 = 0;
							}
							int bytesSent;
							outputBuffer[counter2] = '\0';
							variableForCounting = counter2+1;
							int bytesleft = variableForCounting;
							
							while (totalBytesSent < variableForCounting) {
								if ((bytesSent = send(new_fd, outputBuffer+totalBytesSent, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", bytesSent);
								totalBytesSent += bytesSent;
								bytesleft -= bytesSent;
							}
						} else if (inputBuffer[0] == '2') {
							// Faz match da música que possui o mesmo identificador e reescreve o arquivo de músicas sem ela.
							int identifier;
							char identifierString[1024];
							int counter = 1;
							while (inputBuffer[counter] != '\0') {
								identifierString[counter-1] = inputBuffer[counter];
								counter++;
							}
							identifierString[counter-1] = inputBuffer[counter];
							identifier = atoi(identifierString);
							char identifierWithPrefix[1024];
							char buf2[4096];
							int counter2 = 0, counter3 = 0, whereToResetCounter2 = 0;
							snprintf(identifierWithPrefix, 1024, "Identificador Único: %d\n", identifier);
							int lineLength = 0;
							int matchedIdentifier = 0;
							while ((readError = getline(&line, &lineLength, filePointer)) != -1) {
								
								if (startsWith(line, "Identificador Único:")) {
									if (matchedIdentifier) {
										counter2 = whereToResetCounter2;
									}
									whereToResetCounter2 = counter2;
									// i3 = pi3;
									matchedIdentifier = startsWith(line, identifierWithPrefix);
								}
								if (matchedIdentifier) {
									continue;
								}
								
								counter3 = 0;
								while (line[counter3] != '\0') {
									buf2[counter2] = line[counter3];
									counter2++;
									counter3++;
								}	
							}
							buf2[counter2] = '\0'; 
							int counter6 = 0;
							if (counter2) {
								while (counter6 < counter2) {
									outputBuffer[counter6] = buf2[counter6];
									counter6++;
								}
							} else {
								outputBuffer[0] = '\0';
							}
							fclose(filePointer);
							filePointer = fopen("musicas", "wb");
							fwrite(outputBuffer, 1, counter2, filePointer);
						}  else if (inputBuffer[0] == '1') {
							// Copia todo o arquivo de músicas e adiciona a nova música se já não houver outra música com o mesmo identificador
							// con = identificador + informações
							char content[1024];
							int identifier;
							// idc = "identificador" + o número
							char identifierString[1024];
							int index = 1;
							while (inputBuffer[index] != ' ') {
								content[index-1] = inputBuffer[index];
								index++;
							}
							content[index-1] = inputBuffer[index];
							index++;
							int index2 = 0;
							while (inputBuffer[index] != '\n') {
								identifierString[index2] = inputBuffer[index];
								content[index-1] = inputBuffer[index];
								index++;
							}
							while (inputBuffer[index] != '\0') {
								content[index-1] = inputBuffer[index];
								index++;
							}
							identifierString[index-1] = inputBuffer[index];
							content[index-1] = inputBuffer[index];
							identifier = atoi(identifierString);
							int len = 0;
							char identifierWithPrefix[1024];
							char buf2[4096];
							int counter8 = 0, counter7 = 0, pi3 = 0;
							// Junta string "identificador" com o número
							snprintf(identifierWithPrefix, 1024, "Identificador Único: %d\n", identifier);
							int identifierRegisteredBefore = 0;
							while ((readError = getline(&line, &len, filePointer)) != -1) {
								
								if (startsWith(line, "Identificador Único:")) {
									identifierRegisteredBefore = startsWith(line, identifierWithPrefix);
									if (identifierRegisteredBefore) {
										break;
									}
								}
								
								counter7 = 0;
								while (line[counter7] != '\0') {
									buf2[counter8] = line[counter7];
									counter8++;
									counter7++;
								}	
							}
							if (identifierRegisteredBefore) {
								// Já encontrada música com mesmo identificador
							} else {
								int counter9 = 0;
								buf2[counter8] = '\n';
								counter8++;
								buf2[counter8] = '\n';
								counter8++;
								while (content[counter9] != '\0') {
									buf2[counter8] = content[counter9];
									counter8++;
									counter9++;
								}
								buf2[counter8] = '\0'; 
								counter9 = 0;
								if (counter8) {
									while (counter9 < counter8) {
										outputBuffer[counter9] = buf2[counter9];
										counter9++;
									}
								} else {
									outputBuffer[0] = '\0';
								}
								fclose(filePointer);
								filePointer = fopen("musicas", "wb");
								fwrite(outputBuffer, 1, counter8, filePointer);
							}
							

							
							
						} 
						fclose(filePointer);	
					}


					close(new_fd);
					exit(0);
				}
				 close(new_fd);
				
			} 
			if (FD_ISSET(udpSocket, &readfds)) {
				

				bytesReceivedOrSent = recvfrom(udpSocket, inputBuffer, 1024, 0, ((struct sockaddr *)&udpClient_addr), &udpClientaddr_len);
				if (bytesReceivedOrSent == -1) {
					/* Potencialmente nada para receber
						- Select acionou quando não deveria
					*/
					continue;
				}
				totalBytesReceivedOrSent += bytesReceivedOrSent;
				// bytesleft -= n;
				counter2 = 0;
				counter = 1;
				char identifierString[1024];
				int identifier;
				while (counter2 < totalBytesReceivedOrSent) {
					if (inputBuffer[counter2] == '\0') {
						while (inputBuffer[counter] != '\0') {
							identifierString[counter-1] = inputBuffer[counter];
							counter++;
						}
						identifierString[counter-1] = inputBuffer[counter];
						identifier = atoi(identifierString);
						

						char buf2[1024];

						FILE *fptr;
						char filename[13];
						filename[0] = '\0';
						snprintf(filename, 13, "%d.mp3", identifier);
						fptr = fopen(filename, "rb");
						if (fptr != NULL) {
							// fgets(bufout, 4096, fptr);
							long fileSize;
							char* line = NULL;
							int len = 0;
							ssize_t readError;

							fseek(fptr, 0, SEEK_END);
							fileSize = ftell(fptr);
							int rate = 1000;
							int highestSequenceControlNumber = fileSize / 1000;
							
							char* musicContent = malloc((fileSize) * sizeof(char));
							fseek(fptr, 0, SEEK_SET);
							rewind(fptr);
							fread(musicContent, 1, fileSize, fptr);
							// size = size + 6*(am+1);
							int totalMusicBytesSent = 0;
							int bytesleft = fileSize;
							
							int counter10 = 0;

							int totalBytesSent = 0;
							while (counter10 <= highestSequenceControlNumber) {
								// if (counter10 == 749) {
								// 	printf("opa\n");
								// }
								int sequenceControlStringLength = 7;
								char sequenceControl[sequenceControlStringLength];
								sprintf(sequenceControl, "%06d", counter10);
								char sequenceControlWithMusicContent[6 + min(bytesleft, rate)];
								memcpy(sequenceControlWithMusicContent, sequenceControl, 7);
								memcpy(sequenceControlWithMusicContent+6, musicContent+totalMusicBytesSent, min(bytesleft, rate));
								int total2 = 0, bytesleft2 = 6 + min(bytesleft, rate);
								int len2 = bytesleft2;
								// Envia pacotes de tamanho máximo bytelefts2 = 1006 para evitar desalinhamento e consequentes descartes
								while (total2 < len2) {
								
									if ((bytesReceivedOrSent = sendto(udpSocket, sequenceControlWithMusicContent, bytesleft2, 0, (struct sockaddr*)&udpClient_addr, udpClientaddr_len)) == -1 ) {
										// printf("Erro no envio n=-1 op8 \n");
										//skip packet
										total2 = len2;
										bytesleft2 = 0;
										
										continue;
									}
								
									totalBytesSent += bytesReceivedOrSent;
									if (bytesReceivedOrSent != 1006) {
										// printf("opa\n");
									}
									printf("Enviados %d bytes \n", bytesReceivedOrSent);
									total2 += bytesReceivedOrSent;
									bytesleft2 -= bytesReceivedOrSent;
								}
								printf("%d\n", counter10);
								printf("%d \n", bytesleft);
								counter10++;
								totalMusicBytesSent += total2 - 6;
								bytesleft -= total2 - 6;
								
							}
							free(musicContent);
							printf("tt: %d \n", totalBytesSent);
							
						}
						fclose(fptr);
						totalBytesReceivedOrSent = 0;
						inputBuffer[0] = '\0';
						inputBuffer[1] = '\0';
						counter2 = 0;
						counter = 1;
						break;
					}
					counter2++;
				}
			}
		}

	}
	return 0;
}

