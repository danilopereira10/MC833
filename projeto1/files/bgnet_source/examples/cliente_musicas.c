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

#define MYPORT "3490"
int main(int argc, char* argv[])
{
	struct addrinfo hints, *res;
    int sockfd;

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // AF_INET, AF_INET6, or AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM or SOCK_DGRAM

    getaddrinfo("www.example.com", "3490", &hints, &res);

    // make a socket using the information gleaned from getaddrinfo():
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


	connect(sockfd, res->ai_addr, res->ai_addrlen);
	
	int op = 0;
	while (op < 1 && op > 8) {
		printf("Selecione a operação: \n");
		printf("1 - Cadastrar uma nova música utilizando um identificador \n");
		printf("2 - Remover uma música a partir de seu identificador \n");
		printf("3 - Listar todas as músicas (identificador, título e intérprete) lançadas em um determinado ano \n");
		printf("4 - Listar todas as músicas (identificador, título e intérprete) em um dado idioma lançadas em um dado ano \n");
		printf("5 - Listar todas as músicas de um certo tipo \n");
		printf("6 - Listar todas as informações de uma música dado o seu identificador \n");
		printf("7 - Listar todas as informações de todas as músicas \n");
		printf("8 - fazer um download de uma música a partir de seu identificador \n");
		scanf("%d", &op);
	}
	int op = atoi(argv[1]); 
	int i = 0;
	char buffer[1024];
	buffer[0] = '0' + op;
	if ((op!= 7)) {
		char* content = argv[2];
		while (content[i] != '\0') {
			buffer[i+1] = content[i];
			i++;
		}
	}
	i++;
	buffer[i] = '\0';
	int total = 0;
	int bytesleft = i + 1, len = i + 1;
	int n;
	while (total < len) {
		n  = send(sockfd, buffer+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	

	
	
	

	return 0;
}

