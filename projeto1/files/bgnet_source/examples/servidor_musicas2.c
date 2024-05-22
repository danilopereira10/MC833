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

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int startsWith(const char *a, const char *b) {
	if (strlen(a) == strlen(b)) {
		if (strncmp(a,b, strlen(b)) == 0) {
			return 1;
		}
	}
	return 0;
}

int startsWith2(const char *a, const char *b) {
	if (strlen(a) >= strlen(b)) {
		if (strncmp(a,b, strlen(b)) == 0) {
			return 1;
		}
	}
	return 0;
}

#define MYPORT "3490"
#define MAXBUFLEN 100
int main(void)
{
	// stream sockets and recv()

	struct addrinfo hints, *res;
	int sockfd;
	char buf[1024];
	char bufout[4096];
	int n;
    
    struct sockaddr_storage their_addr;

	// get host info, make socket, and connect it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// getaddrinfo("www.example.com", "3490", &hints, &res);

	int rv;
    // if ((rv = getaddrinfo(NULL, "3490", &hints, &res)) != 0 ) {
    //     fprintf(stderr, "getaddrinfo tcp: %s \n", gai_strerror(rv));
    // }

    if ((rv = getaddrinfo(NULL, "3490", &hints, &res)) != 0 ) {
        fprintf(stderr, "getaddrinfo tcp: %s \n", gai_strerror(rv));
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("tcp socket \n");
            continue;
        }
        break;
    }
    
	bind(sockfd, p->ai_addr, p->ai_addrlen);
	listen(sockfd, 10);
    socklen_t addr_size;
    addr_size = sizeof their_addr;
	int new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
	n = recv(new_fd, buf, 1024, 0);
    // while(1) {
    //     int new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &addr_size);
    //     pid_t pid;
    //     if ((pid = fork()) == 0) {
    //         close(sockfd);
    //         int dgram_socket;
	// 		// struct addrinfo dhints, *dservinfo, *pd;
	// 		// int drv;
	// 		// int dnumbytes;
	// 		// struct sockaddr_storage dtheir_addr;
	// 		// char dbuf[MAXBUFLEN];
	// 		// socklen_t daddr_len;
	// 		// char ds[INET6_ADDRSTRLEN];

	// 		// memset(&dhints, 0, sizeof dhints);
	// 		// dhints.ai_family = AF_INET6;
	// 		// dhints.ai_socktype = SOCK_DGRAM;
	// 		// dhints.ai_flags = AI_PASSIVE;

	// 		// all right! now that we're connected, we can receive some data!
	// 		int total = 0;
	// 		int bytesleft = 1024, len = 1024;
	// 		int j = 0;
	// 		while (1) {
	// 			int c = 0;
	// 			int f = 0;
	// 			int f2 = 0;
	// 			while (1) {
	// 				n = recv(new_fd, buf+total, bytesleft, 0);
	// 				printf("recv()'d %d bytes of data in buf\n", n);
	// 				if ((n == -1)) { 
	// 					exit(1);  
	// 				} else if (n == 0) {
	// 					f2 = 1;
	// 					break;
	// 				} else {
	// 					total += n;
	// 					bytesleft -= n;
	// 					while (j < total) {
	// 						if ((buf[j] == '\0')) {
	// 							total = 0;
	// 							bytesleft = 1024;
	// 							c++;
	// 						}
	// 						j++;
	// 						if (c && buf[0] != '4') {
	// 							f = 1;
	// 							break;
	// 						} else if (c == 2) {
	// 							f = 1;
	// 							break;
	// 						}
	// 					}
	// 				}
	// 				if (f || f2) {
	// 					break;
	// 				}
	// 			}
	// 			if (f2) {
	// 				continue;
	// 			}

	// 			FILE *fptr;
	// 			fptr = fopen("musicas", "r");;
	// 			// fgets(bufout, 4096, fptr);
	// 			long size;
	// 			char* line = NULL;
	// 			size_t len = 0;
	// 			ssize_t read;
	// 			if (buf[0] == '7') {
	// 				fseek(fptr, 0, SEEK_END);
	// 				size = ftell(fptr);
	// 				fseek(fptr, 0, SEEK_SET);
	// 				rewind(fptr);
	// 				fread(bufout, 1, size, fptr);
    //                 int total = 0;
                    
    //                 int n;
    //                 bufout[size] = '\0';
    //                 int len = size+1;
    //                 int bytesleft = len;
                    
    //                 while (total < len) {
    //                     if ((n = send(new_fd, bufout+total, bytesleft, 0)) == -1 ) {
    //                         printf("Erro no envio n=-1 op7 \n");
    //                     }
    //                     printf("Enviados %d bytes \n", n);
    //                     total += n;
    //                     bytesleft -= n;
    //                 }
	// 			} else if (buf[0] == '6') {
	// 				int id;
	// 				char idc[1024];
	// 				int i2 = 1;
	// 				while (buf[i2] != '\0') {
	// 					idc[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				idc[i2-1] = buf[i2];
	// 				id = atoi(idc);
	// 				char idc2[1024];
	// 				char buf2[1024];
	// 				int i3 = 0, i4 = 0, pi3 = 0;
	// 				snprintf(idc2, 1024, "Identificador único: %d", id);
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
	// 					if (startsWith2(line, "Identificador único:")) {
	// 						if (c2) {
	// 							pi3 = i3;
	// 						}
	// 						i3 = pi3;
	// 						c2 = startsWith(line, idc2);
	// 					}
						
	// 					i4 = 0;
	// 					while (line[i4] != '\0') {
	// 						buf2[i3] = line[i4];
	// 						i3++;
	// 						i4++;
	// 					}	
	// 				}
	// 				buf2[i3] = '\0'; 
	// 				int i5 = 0;
	// 				while (i5 < i3) {
	// 					bufout[i5] = buf2[i5];
	// 					i5++;
	// 				}
	// 				if (i5 == 0) {
	// 					bufout[0] = '\0';
	// 				}
					

	// 			} else if (buf[0] == '5') {
					
	// 				char tipo[1024];
	// 				int i2 = 1;
	// 				while (buf[i2] != '\0') {
	// 					tipo[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				tipo[i2-1] = buf[i2];
					
	// 				char tipo2[1024];
	// 				char buf2[1024];
	// 				int i3 = 0, i4 = 0, pi3 = 0;
	// 				snprintf(tipo2, 1024, "Tipo de música: %s", tipo);
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
	// 					if (startsWith2(line, "Tipo de música:")) {
	// 						if (c2) {
	// 							pi3 = i3;
	// 						}
	// 						i3 = pi3;
	// 						c2 = startsWith(line, tipo2);
	// 					}
						
	// 					if (startsWith2(line, "Identificador único")|| startsWith2(line, "Título") || startsWith2(line, "Intérprete")) {
	// 						i4 = 0;
	// 						while (line[i4] != '\0') {
	// 							buf2[i3] = line[i4];
	// 							i3++;
	// 							i4++;
	// 						}
	// 					}	
	// 				}
	// 				buf2[i3] = '\0'; 
	// 				int i5 = 0;
	// 				if (i3) {
	// 					while (i5 < i3) {
	// 						bufout[i5] = buf2[i5];
	// 						i5++;
	// 					}
	// 				} else {
	// 					bufout[0] = '\0';
	// 				}
	// 			} else if (buf[0] == '4') {
					
	// 				char ano[1024];
	// 				int i2 = 1;
	// 				while (buf[i2] != '\0') {
	// 					ano[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				ano[i2-1] = buf[i2];
					
	// 				char ano2[1024];
	// 				char buf2[1024];
	// 				int i3 = 0, i4 = 0, pi3 = 0;
	// 				int anoint = atoi(ano);
	// 				snprintf(ano2, 1024, "Ano de lançamento: %d", anoint);
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
	// 					if (startsWith2(line, "Ano de lançamento:")) {
	// 						if (c2) {
	// 							pi3 = i3;
	// 						}
	// 						i3 = pi3;
	// 						c2 = startsWith(line, ano2);
	// 					}
						

	// 					if (startsWith2(line, "Identificador único")|| startsWith2(line, "Título") || startsWith2(line, "Intérprete")) {
	// 						i4 = 0;
	// 						while (line[i4] != '\0') {
	// 							buf2[i3] = line[i4];
	// 							i3++;
	// 							i4++;
	// 						}
	// 					}	
	// 				}
	// 				buf2[i3] = '\0'; 
	// 				int i5 = 0;
	// 				if (i3) {
	// 					while (i5 < i3) {
	// 						bufout[i5] = buf2[i5];
	// 						i5++;
	// 					}
	// 				} else {
	// 					bufout[0] = '\0';
	// 				}
	// 			} else if (buf[0] == '3') {
					
	// 				char ano[1024];
	// 				char idioma[1024];
	// 				int i2 = 1;
	// 				int i3 = 0, i4 = 0;
	// 				while (buf[i2] != '\0') {
	// 					idioma[i3] = buf[i2];
	// 					i2++;
	// 					i3++;
	// 				}
	// 				idioma[i3] = buf[i2];
	// 				i2++;

	// 				while (buf[i2] != '\0') {
	// 					ano[i4] = buf[i2];
	// 					i2++;
	// 					i4++;
	// 				}
	// 				ano[i4] = buf[i2];
					
	// 				char ano2[1024];
	// 				char idioma2[1024];
	// 				char buf2[1024];
	// 				i3 = 0, i4 = 0;
	// 				int pi3 = 0;
	// 				int anoint = atoi(ano);
	// 				snprintf(ano2, 1024, "Ano de lançamento: %d", anoint);
	// 				snprintf(idioma2, 1024, "Idioma: %s", idioma);
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
	// 					if (startsWith2(line, "Idioma:")) {
	// 						c2 = 0;
	// 						if (startsWith(line, idioma2)) {
	// 							c2++;
	// 						}
	// 					} else if (startsWith2(line, "Ano de lançamento:")) {	
	// 						if (startsWith(line, ano2) && c2) {
	// 							c2++;
	// 						} else {
	// 							c2 = 0;
	// 						}
	// 					}
	// 					if (c2 == 2) {
	// 						pi3 = i3;
	// 					} else {
	// 						i3 = pi3;
	// 					}

	// 					if (startsWith2(line, "Identificador único")|| startsWith2(line, "Título") || startsWith2(line, "Intérprete")) {
	// 						i4 = 0;
	// 						while (line[i4] != '\0') {
	// 							buf2[i3] = line[i4];
	// 							i3++;
	// 							i4++;
	// 						}
	// 					}	
	// 				}
	// 				buf2[i3] = '\0'; 
	// 				int i5 = 0;
	// 				if (i3) {
	// 					while (i5 < i3) {
	// 						bufout[i5] = buf2[i5];
	// 						i5++;
	// 					}
	// 				} else {
	// 					bufout[0] = '\0';
	// 				}
	// 			} else if (buf[0] == '2') {
	// 				int id;
	// 				char idc[1024];
	// 				int i2 = 1;
	// 				while (buf[i2] != '\0') {
	// 					idc[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				idc[i2-1] = buf[i2];
	// 				id = atoi(idc);
	// 				char idc2[1024];
	// 				char buf2[4096];
	// 				int i3 = 0, i4 = 0, pi3 = 0;
	// 				snprintf(idc2, 1024, "Identificador único: %d", id);
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
						
	// 					if (startsWith2(line, "Identificador único:")) {
	// 						if (c2) {
	// 							pi3 = i3;
	// 						}
	// 						// i3 = pi3;
	// 						c2 = startsWith(line, idc2);
	// 					}
	// 					if (c2) {
	// 						continue;
	// 					}
						
	// 					i4 = 0;
	// 					while (line[i4] != '\0') {
	// 						buf2[i3] = line[i4];
	// 						i3++;
	// 						i4++;
	// 					}	
	// 				}
	// 				buf2[i3] = '\0'; 
	// 				int i5 = 0;
	// 				if (i3) {
	// 					while (i5 < i3) {
	// 						bufout[i5] = buf2[i5];
	// 						i5++;
	// 					}
	// 				} else {
	// 					bufout[0] = '\0';
	// 				}
	// 				fclose(fptr);
	// 				fptr = fopen("musicas", "w");
	// 				fputs(bufout, fptr);
	// 			}  else if (buf[0] == '1') {
	// 				char con[1024];
	// 				int id;
	// 				char idc[1024];
	// 				int i2 = 1;
	// 				while (buf[i2] != '\n') {
	// 					idc[i2-1] = buf[i2];
	// 					con[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				while (buf[i2] != '\0') {
	// 					con[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				idc[i2-1] = buf[i2];
	// 				con[i2-1] = buf[i2];
	// 				id = atoi(idc);

	// 				char idc2[1024];
	// 				char buf2[4096];
	// 				int i3 = 0, i4 = 0, pi3 = 0;
	// 				snprintf(idc2, 1024, "Identificador único: %d", id);
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
						
	// 					if (startsWith2(line, "Identificador único:")) {
	// 						c2 = startsWith(line, idc2);
	// 						if (c2) {
	// 							break;
	// 						}
	// 					}
						
	// 					i4 = 0;
	// 					while (line[i4] != '\0') {
	// 						buf2[i3] = line[i4];
	// 						i3++;
	// 						i4++;
	// 					}	
	// 				}
	// 				if (c2) {
	// 					//
	// 				} else {
	// 					int i5 = 0;
	// 					while (con[i5] != '\0') {
	// 						buf2[i3] = con[i5];
	// 						i3++;
	// 						i5++;
	// 					}
	// 					buf2[i3] = '\0'; 
	// 					i5 = 0;
	// 					if (i3) {
	// 						while (i5 < i3) {
	// 							bufout[i5] = buf2[i5];
	// 							i5++;
	// 						}
	// 					} else {
	// 						bufout[0] = '\0';
	// 					}
	// 					fclose(fptr);
	// 					fptr = fopen("musicas", "w");
	// 					fputs(bufout, fptr);
	// 				}
					

					
					
	// 			} else {
	// 				int id;
	// 				char idc[1024];
	// 				int i2 = 1;
	// 				while (buf[i2] != '\0') {
	// 					idc[i2-1] = buf[i2];
	// 					i2++;
	// 				}
	// 				idc[i2-1] = buf[i2];
	// 				id = atoi(idc);
	// 				char idc2[1024];
	// 				char buf2[1024];
	// 				int i3 = 0, i4 = 0, pi3 = 0;
	// 				snprintf(idc2, 1024, "%d.mp3", id);

	// 				FILE* fptr2 = fopen(idc2, "r");
	// 				int c2 = 0;
	// 				while ((read = getline(&line, &len, fptr)) != -1) {
	// 					if (startsWith2(line, "Identificador único:")) {
	// 						if (c2) {
	// 							pi3 = i3;
	// 						}
	// 						i3 = pi3;
	// 					}
	// 					c2 = startsWith(line, idc2);
	// 					i4 = 0;
	// 					while (line[i4] != '\0') {
	// 						buf2[i3] = line[i4];
	// 						i3++;
	// 						i4++;
	// 					}	
	// 				}
	// 				buf2[i3] = '\0'; 
	// 				int i5 = 0;
	// 				while (i5 < i3) {
	// 					bufout[i5] = buf2[i5];
	// 					i5++;
	// 				}
	// 			}
	// 			fclose(fptr);	
	// 		}


	// 		close(new_fd);
	// 		exit(0);
	// 	}
	// 	close(new_fd);
    // }
    

	
		
	

	
	return 0;
}

