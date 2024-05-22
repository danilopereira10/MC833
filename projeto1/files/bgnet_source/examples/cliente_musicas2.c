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

	int dcsockfd;
	struct addrinfo dchints, *dcservinfo, *dcp;
	int dcrv;
	int dcnumbytes;
	struct sockaddr_storage dctheir_addr;
	socklen_t dcaddr_len;


	memset(&dchints, 0, sizeof dchints);
	dchints.ai_family = AF_UNSPEC;
	dchints.ai_socktype = SOCK_DGRAM;
	dchints.ai_flags = AI_PASSIVE;

	if ((dcrv = getaddrinfo(serverip, portudp, &dchints, &dcservinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(dcrv));
		return 1;
	}

	for (dcp = dcservinfo; dcp != NULL; dcp = dcp->ai_next) {
		if ((dcsockfd = socket(dcp->ai_family, dcp->ai_socktype, dcp->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		if ((bind(dcsockfd, dcp->ai_addr, dcp->ai_addrlen)) == -1) {
			close(dcsockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (dcp == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	

    // make a socket using the information gleaned from getaddrinfo():
    // sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


	connect(sockfd, p->ai_addr, p->ai_addrlen);
	
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
		if ((op!= 7)) {
			char* content = NULL;
			if ((op == 2) || (op == 6) || (op == 8)) {
				printf("Digite o identificador: \n");
			} else if (op == 3) {
				printf("Digite o ano: \n");
			} else if (op == 4) {
				printf("Digite o idioma: \n");
			} else if (op == 1) {
				printf("Coloque todas as informações da música: \n");
			} else {
				printf("Digite o tipo: \n");
			}
			
			scanf("%ms", &content);
			//don't forget to free content
			while (content[i] != '\0') {
				buffer[i+1] = content[i];
				i++;
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
			} else {
				int i4 = 0;
				i++;
				while (serverip[i4] != '\0') {
					buffer[i] = serverip[i4];
					i++;
				}
				buffer[i] = '\0';

				while (total < len) {
					n = sendto(dsockfd, buffer+total, bytesleft, 0, dp->ai_addr, dp->ai_addrlen);
					if (n == -1) {
						printf("erro n=-1 no op 8");
						break;
					}
					total += n;
					bytesleft -= n;
				}
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

