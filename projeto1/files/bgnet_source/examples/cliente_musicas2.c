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
	// char* clientudpport = argv[4];

	struct addrinfo hints, *res;
    int sockfd;
    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // AF_INET, AF_INET6, or AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM or SOCK_DGRAM


    // getaddrinfo("www.example.com", "3490", &hints, &res);
    int rv;
	// if ((rv = getaddrinfo("oliveira.lab.ic.unicamp.br", "3490", &hints, &res)) != 0) {
    //     fprintf(stderr, "getaddrinfo tcp: %s\n", gai_strerror(rv));
    // }
    if ((rv = getaddrinfo(serverip, porttcp, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo tcp: %s\n", gai_strerror(rv));
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("tcp socket \n");
            continue;
        }
        break;
    }


	int dsockfd;
	struct addrinfo dhints, *dservinfo, *dp;
	int drv;
	int dnumbytes;

	memset(&dhints, 0, sizeof dhints);
	dhints.ai_family = AF_UNSPEC;
	dhints.ai_socktype = SOCK_DGRAM;

	if ((drv = getaddrinfo(serverip, portudp, &dhints, &dservinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(drv));
		return 1;
	}

	for (dp = dservinfo; dp != NULL; dp = dp->ai_next) {
		if ((dsockfd = socket(dp->ai_family, dp->ai_socktype, dp->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}

	if (dp == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}



	

    // make a socket using the information gleaned from getaddrinfo():
    // sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


	connect(sockfd, p->ai_addr, p->ai_addrlen);
	
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	while (1) {
		int op = 0;
		while (op < 1 || op > 9) {
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
			scanf("%d", &op);
		}
		if (op == 9) {
			break;
		}
		// int op = atoi(argv[1]); 
		int i = 0;
		char buffer[1024];
		buffer[0] = '0' + op;
		char filename[14];
		if ((op!= 7)) {
			char content[1024];
			if ((op == 2) || (op == 6) || (op == 8)) {
				printf("Digite o identificador: \n");
			} else if (op == 3) {
				printf("Digite o ano: \n");
			} else if (op == 4) {
				printf("Digite o idioma: \n");
			} else if (op == 1) {
				printf("Coloque todas as informações da música (e digite Tab e aperte Enter no final): \n");
			} else {
				printf("Digite o tipo: \n");
			}
			
			if (op != 1) {
				scanf("%s", content);
			} else {
				scanf("%[^\t]",content);
			}
			//don't forget to free content
			while (content[i] != '\0') {
				buffer[i+1] = content[i];
				i++;
			}
			if (op == 8) {
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
			// int f2 = 0;
			if (op == 4) {
				buffer[i+1] = '\0';
				i++;
				printf("Digite o ano: \n");
				scanf("%ms", &content2);
				// f2 = 1;

				int j = 0;
				while (content2[j] != '\0') {
					buffer[i+1] = content2[j];
					i++;
					j++;
				}
			}
			free(content2);
			
			

			i++;
			buffer[i] = '\0';
			int total = 0;
			int bytesleft = i + 1, len = i + 1;
			int n;
			if (op != 8) {
				while (total < len) {
					n  = send(sockfd, buffer+total, bytesleft, 0);
					if (n == -1) {
						printf("erro: n=-1 no op entre 1 e 6\n"); 
						break; }
					total += n;
					bytesleft -= n;
				}
				char bufin[4096];
				bufin[0] = '\0';
				total = 0;
				bytesleft = 4096;
				int j = 0;
				int f2 = 0;
				if ((op != 2) && (op != 1)) {
					while (total < 4096) {
						n = recv(sockfd, bufin+total, bytesleft, 0);
						if (n == 0) {
							break;
						}
						total += n;
						bytesleft -= n;
						while (j < total) {
							if (bufin[j] == '\0') {
								f2 = 1;
								break;
							}
							j++;
						}
						if (f2) {
							break;
						}
					}
					printf(bufin);
				}
			} else {
				while (total < len) {
					n = sendto(dsockfd, buffer+total, bytesleft, 0, dp->ai_addr, dp->ai_addrlen);
					if (n == -1) {
						printf("erro n=-1 no op 8");
						break;
					}
					total += n;
					bytesleft -= n;
				}
				fd_set readfds;
				int total2 = 0;
				char* bufin = malloc(1024*1024*10 * sizeof(char));
				bytesleft = 1024*1024*10;
				bufin[bytesleft-1] = '\0';
				int rate = 1000;
				char buft[rate];
				int high = 0;
				int hs = 0;
				while (1) {
					struct timeval tv;
					FD_ZERO(&readfds);
					FD_SET(dsockfd, &readfds);
					int n2 = dsockfd+1;
					tv.tv_sec =1;
					tv.tv_usec = 0;
					drv = select(n2, &readfds, NULL, NULL, &tv);
					if (drv == -1) {
						perror("select");
						bufin[total2] = '\0';
						break;
					} else if (drv == 0) {
						bufin[total2] = '\0';
						break;
						//printf("Timeout occurred! No data after 1 second. \n");
					} else {
						int n3 = recvfrom(dsockfd, buft, rate, 0, (struct sockaddr *)&their_addr, &addr_len);
						int t = 0;
						for (int i = 0; (i < 6) && (i < n3); i++) {
							if ((buft[i] >'9') || (buft[i] < '0')) {
								t = 1;
								break;
							}
						}
						if ((n3 <= 7) || t) {
							continue;
						}
						char n4[7];
						memcpy(n4, buft, 6);
						n4[6] = '\0';
						int seq = (int) strtol(n4, (char**) NULL, 10);
						int i4 = 0;
						hs = max(hs, seq);
						

						while ((i4 + 6) < n3-1) {
							if (seq * rate + i4 < 1024*1024*10-1) {
								bufin[seq * rate + i4] = buft[i4 + 6];
							}
							high = max(high, seq*rate+i4);
							i4++;
						}
					}

				}
				FILE* fptr = fopen(filename, "wb");
				high++;
				fwrite(bufin, 1, high, fptr);
				fclose(fptr);
				free(bufin);
			}
			
		} else {
			buffer[0] = '7';
			buffer[1] = '\0';
			int total = 0;
			int bytesleft = 2;
			int len = 2;
			int n;
			while (total < len) {
				n = send(sockfd, buffer+total, bytesleft, 0);
				if (n == -1) {
					printf("erro n=-1 no envio op 7 \n");
					break;
				}
                printf("enviados %d bytess \n", n);
				total += n;
				bytesleft -= n;
			}
            char bufin[4096];
			bufin[0] = '\0';
            total = 0;
            bytesleft = 4096;
            int j = 0;
			int f2 = 0;
            while (total < 4096) {
                n = recv(sockfd, bufin+total, bytesleft, 0);
                total += n;
                bytesleft -= n;
                while (j < total) {
                    if (bufin[j] == '\0') {
						f2 = 1;
                        break;
                    }
                    j++;
                 }
				 if (f2) {
					break;
				 }
            }
            printf(bufin);
            
		}
	}

	
	
	

	return 0;
}

