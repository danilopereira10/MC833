/*
** server.c -- a stream socket server demo
*/

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
	

	struct addrinfo hints, *res;
	int sockfd;
	char buf[1024];
	char bufout[4096];
	int n;
    
    struct sockaddr_storage their_addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	int rv;
    
	char* porttcp = argv[1];
	char* portudp = argv[2];
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &res)) != 0 ) {
        fprintf(stderr, "getaddrinfo tcp: %s \n", gai_strerror(rv));
    }

    struct addrinfo *p;
	//tentativa de criação e binding de socket
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("tcp socket \n");
            continue;
        }
		int yes = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			printf("erro no setsockopt \n");
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			printf("erro no bind \n");
			continue;
		}
        break;
    }
    
	
	
	if (listen(sockfd, 10) == -1) {
		printf("erro no listen \n");
	}
    socklen_t addr_size;
    addr_size = sizeof their_addr;

	/* socket para UDP. Abaixo seguem todas as variáveis e criações 
		correspondentes.
	*/
	int dsockfd;
	struct addrinfo dhints, *dservinfo, *dp;
	int drv;
	int dnumbytes;
	struct sockaddr_storage dtheir_addr;
	// char dbuf[MAXBUFLEN];
	socklen_t daddr_len;
	char ds[INET6_ADDRSTRLEN];

	memset(&dhints, 0, sizeof dhints);
	dhints.ai_family = AF_UNSPEC;
	dhints.ai_socktype = SOCK_DGRAM;
	dhints.ai_flags = AI_PASSIVE;

	if ((drv = getaddrinfo(NULL, argv[2], &dhints, &dservinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(drv));
		return 1;
	}

	
	//Tentativa de criação socket UDP
	for (dp = dservinfo; dp != NULL; dp=dp->ai_next) {
		if ((dsockfd = socket(dp->ai_family, dp->ai_socktype, dp->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		int yes2 = 1;
		if (setsockopt(dsockfd, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof yes2) == -1) {
			printf("erro no setsockopt 2\n");
		}

		if ((bind(dsockfd, dp->ai_addr, dp->ai_addrlen)) == -1) {
			close(dsockfd);
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
	
	daddr_len = sizeof dtheir_addr;

	
	/* Tornar socket UDP não bloqueante.
		No manual do Beej.s diz que o select pode dizer que o socket
		está pronto para "recv" no Linux mesmo sem estar. Eles sugerem
		o método fcntl para evitar o bloqueio do socket. Só não fiz isso para 
		TCP também porque a entrega é confiável. Entretanto, meu código cliente pressupõe
		que será possível conectar à porta TCP do servidor.
	*/

	fcntl(dsockfd, F_SETFL, O_NONBLOCK);

	fd_set readfds;

	int i4 = 0;
	/*O símbolo '\0' é utilizado para reconhecer o fim de entradas. 
	*/
	
	buf[0] = '\0';
	buf[1] = '\0';



	
	int bytesleft = 1024;
	int total = 0;
	int i5 = 0;

	int id;
	char idc[1024];
	int i2 = 1;

	
	while (1) {
		
		/* Preparação de variáveis para select
			O select atua entre receber conexões na porta TCP ou
			receber requisições pela porta UDP
		*/
		struct timeval tv;
		FD_ZERO(&readfds);

		FD_SET(sockfd, &readfds);
		FD_SET(dsockfd, &readfds);
		int n2 = max(dsockfd,sockfd) + 1;
		//Intervalo de 1 segundo.
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		drv = select(n2, &readfds, NULL, NULL, &tv);
		if (drv == -1) {
			//Nenhum dado
			perror("select");
			total = 0;
			bytesleft = 1024;
			i5 = 0;
			i2 = 1;
		} else if (drv == 0) {
			//printf("Timeout occurred! No data after 1 second. \n");
		} else {
			if (FD_ISSET(sockfd, &readfds)) {

				int new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
				printf("Recebida conexão \n");
				pid_t pid;
				
				if ((pid = fork()) == 0) {
					close(sockfd);
					
					
					while (1) {
						int total = 0;
						int bytesleft = 1024; 
						size_t len = 1024;
						int j = 0;
						int c = 0;
						int f = 0;
						int f2 = 0;
						while (1) {
							n = recv(new_fd, buf+total, bytesleft, 0);
							printf("recv()'d %d bytes of data in buf\n", n);
							if ((n == -1)) { 
								// exit(1);  
							} else if (n == 0) {
								f2 = 1;
								break;
							} else {
								total += n;
								bytesleft -= n;
								while (j < total) {
									if ((buf[j] == '\0')) {
										total = 0;
										bytesleft = 1024;
										c++;
									}
									j++;
									if (c && buf[0] != '4') {
										f = 1;
										break;
									} else if (c == 2) {
										f = 1;
										break;
									}
								}
							}
							if (f || f2) {
								break;
							}
						}
						if (f2) {
							break;
						}

						FILE *fptr;
						fptr = fopen("musicas", "r");;
						// fgets(bufout, 4096, fptr);
						long size;
						char* line = NULL;
						len = 0;
						ssize_t read;
						if (buf[0] == '7') {
							fseek(fptr, 0, SEEK_END);
							size = ftell(fptr);
							fseek(fptr, 0, SEEK_SET);
							rewind(fptr);
							fread(bufout, 1, size, fptr);
							int total = 0;
							
							int n;
							bufout[size] = '\0';
							int len = size+1;
							int bytesleft = len;
							
							while (total < len) {
								if ((n = send(new_fd, bufout+total, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", n);
								total += n;
								bytesleft -= n;
							}
						} else if (buf[0] == '6') {
							int id;
							char idc[1024];
							int i2 = 1;
							while (buf[i2] != '\0') {
								idc[i2-1] = buf[i2];
								i2++;
							}
							idc[i2-1] = buf[i2];
							id = atoi(idc);
							char idc2[1024];
							char buf2[1024];
							int i3 = 0, i4 = 0, pi3 = 0;
							snprintf(idc2, 1024, "Identificador Único: %d\n", id);
							int c2 = 0;
							while ((read = getline(&line, &len, fptr)) != -1) {
								printf(line);
								if (startsWith(line, "Identificador Único:")) {
									if (c2) {
										pi3 = i3;
										break;
										
									}
									i3 = pi3;
									c2 = startsWith(line, idc2);
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
								bufout[i5] = buf2[i5];
								i5++;
							}
							if (i5 == 0) {
								bufout[0] = '\0';
							}
							
							int n;
							bufout[i3] = '\0';
							int len = i3+1;
							int bytesleft = len;
							
							while (total < len) {
								if ((n = send(new_fd, bufout+total, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", n);
								total += n;
								bytesleft -= n;
							}

						} else if (buf[0] == '5') {
							
							char tipo[1024];
							int i2 = 1;
							while (buf[i2] != '\0') {
								tipo[i2-1] = buf[i2];
								i2++;
							}
							tipo[i2-1] = buf[i2];
							
							char tipo2[1024];
							char buf2[1024];
							int i3 = 0, i4 = 0, pi3 = 0;
							snprintf(tipo2, 1024, "Tipo de música: %s\n", tipo);
							int c2 = 0;
							while ((read = getline(&line, &len, fptr)) != -1) {
								if (startsWith(line, "Tipo de música:")) {
									if (c2) {
										pi3 = i3;
									}
									i3 = pi3;
									c2 = startsWith(line, tipo2);
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
									bufout[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufout[0] = '\0';
							}

							int n;
							bufout[i3] = '\0';
							int len = i3+1;
							int bytesleft = len;
							
							while (total < len) {
								if ((n = send(new_fd, bufout+total, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", n);
								total += n;
								bytesleft -= n;
							}
						} else if (buf[0] == '4') {
							
							char ano[1024];
							int i2 = 1;
							while (buf[i2] != '\0') {
								ano[i2-1] = buf[i2];
								i2++;
							}
							ano[i2-1] = buf[i2];
							
							char ano2[1024];
							char buf2[1024];
							int i3 = 0, i4 = 0, pi3 = 0;
							int anoint = atoi(ano);
							snprintf(ano2, 1024, "Ano de lançamento: %d\n", anoint);
							int c2 = 0;
							while ((read = getline(&line, &len, fptr)) != -1) {
								if (startsWith(line, "Ano de lançamento:")) {
									if (c2) {
										pi3 = i3;
									}
									i3 = pi3;
									c2 = startsWith(line, ano2);
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
									bufout[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufout[0] = '\0';
							}
							int n;
							bufout[i3] = '\0';
							int len = i3+1;
							int bytesleft = len;
							
							while (total < len) {
								if ((n = send(new_fd, bufout+total, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", n);
								total += n;
								bytesleft -= n;
							}
						} else if (buf[0] == '3') {
							
							char ano[1024];
							char idioma[1024];
							int i2 = 1;
							int i3 = 0, i4 = 0;
							while (buf[i2] != '\0') {
								idioma[i3] = buf[i2];
								i2++;
								i3++;
							}
							idioma[i3] = buf[i2];
							i2++;

							while (buf[i2] != '\0') {
								ano[i4] = buf[i2];
								i2++;
								i4++;
							}
							ano[i4] = buf[i2];
							
							char ano2[1024];
							char idioma2[1024];
							char buf2[1024];
							i3 = 0, i4 = 0;
							int pi3 = 0;
							int anoint = atoi(ano);
							snprintf(ano2, 1024, "Ano de lançamento: %d\n", anoint);
							snprintf(idioma2, 1024, "Idioma: %s\n", idioma);
							int c2 = 0;
							while ((read = getline(&line, &len, fptr)) != -1) {
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
								}
								if (c2 == 2) {
									pi3 = i3;
								} else {
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
									bufout[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufout[0] = '\0';
							}

							int n;
							bufout[i3] = '\0';
							int len = i3+1;
							int bytesleft = len;
							
							while (total < len) {
								if ((n = send(new_fd, bufout+total, bytesleft, 0)) == -1 ) {
									printf("Erro no envio n=-1 op7 \n");
								}
								printf("Enviados %d bytes \n", n);
								total += n;
								bytesleft -= n;
							}
						} else if (buf[0] == '2') {
							int id;
							char idc[1024];
							int i2 = 1;
							while (buf[i2] != '\0') {
								idc[i2-1] = buf[i2];
								i2++;
							}
							idc[i2-1] = buf[i2];
							id = atoi(idc);
							char idc2[1024];
							char buf2[4096];
							int i3 = 0, i4 = 0, pi3 = 0;
							snprintf(idc2, 1024, "Identificador Único: %d\n", id);
							int c2 = 0;
							while ((read = getline(&line, &len, fptr)) != -1) {
								
								if (startsWith(line, "Identificador Único:")) {
									if (c2) {
										pi3 = i3;
									}
									// i3 = pi3;
									c2 = startsWith(line, idc2);
								}
								if (c2) {
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
									bufout[i5] = buf2[i5];
									i5++;
								}
							} else {
								bufout[0] = '\0';
							}
							fclose(fptr);
							fptr = fopen("musicas", "w");
							fputs(bufout, fptr);
						}  else {
							char con[1024];
							int id;
							char idc[1024];
							int i2 = 1;
							while (buf[i2] != '\n') {
								idc[i2-1] = buf[i2];
								con[i2-1] = buf[i2];
								i2++;
							}
							while (buf[i2] != '\0') {
								con[i2-1] = buf[i2];
								i2++;
							}
							idc[i2-1] = buf[i2];
							con[i2-1] = buf[i2];
							id = atoi(idc);

							char idc2[1024];
							char buf2[4096];
							int i3 = 0, i4 = 0, pi3 = 0;
							snprintf(idc2, 1024, "Identificador Único: %d", id);
							int c2 = 0;
							while ((read = getline(&line, &len, fptr)) != -1) {
								
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
								//
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
										bufout[i5] = buf2[i5];
										i5++;
									}
								} else {
									bufout[0] = '\0';
								}
								fclose(fptr);
								fptr = fopen("musicas", "w");
								fputs(bufout, fptr);
							}
							

							
							
						} 
						fclose(fptr);	
					}


					close(new_fd);
					exit(0);
				}
				close(new_fd);
				
			} else {
				

				n = recvfrom(dsockfd, buf, bytesleft+total, 0, ((struct sockaddr *)&dtheir_addr), &daddr_len);
				total += n;
				bytesleft -= n;
				i5 = 0;
				i2 = 1;
				while (i5 < total) {
					if (buf[i5] == '\0') {
						while (buf[i2] != '\0') {
							idc[i2-1] = buf[i2];
							i2++;
						}
						idc[i2-1] = buf[i2];
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
							int total = 0;
							bytesleft = size;
							
							int j = 0;


							while (j <= am) {
								int s2 = 7;
								char str[s2];
								sprintf(str, "%06d", j);
								char str2[6 + min(bytesleft, rate)];
								memcpy(str2, str, 6);
								strncat(str2, bufout2+total, min(bytesleft, rate));
								int total2 = 0, bytesleft2 = 6 + min(bytesleft, rate);
								int len2 = bytesleft2;
								while (total2 < len2) {
								
									if ((n = sendto(dsockfd, str2, 6 + min(bytesleft, bytesleft2), 0, (struct sockaddr*)&dtheir_addr, daddr_len)) == -1 ) {
										printf("Erro no envio n=-1 op8 \n");
										j++;
										//skip packet
										total2 = len2;
										bytesleft2 = 0;
										continue;
									}
								
									if (n != 1006) {
										printf("opa\n");
									}
									printf("Enviados %d bytes \n", n);
									total2 += n;
									bytesleft2 -= n;
								}
								total += total2;
								bytesleft -= total2;
								
							}
							free(bufout2);
							
						}
						
						total = 0;
						bytesleft = 1024;
						buf[0] = '\0';
						buf[1] = '\0';
						i5 = 0;
						i2 = 1;
						break;
					}
					i5++;
				}
			}
		}

	}





	// all right! now that we're connected, we can receive some data!
    
    

	
		
	

	
	return 0;
}

