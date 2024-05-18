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
int main(void)
{
	// stream sockets and recv()

	struct addrinfo hints, *res;
	int sockfd;
	char buf[1024];
	char bufout[1024];
	int n;

	// get host info, make socket, and connect it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo("www.example.com", "3490", &hints, &res);
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	connect(sockfd, res->ai_addr, res->ai_addrlen);

	// all right! now that we're connected, we can receive some data!
	int total = 0;
	int bytesleft = 1024, len = 1024;
	int j = 0;
	while (1) {
		int c = 0;
		int f = 0;
		while (1) {
			n = recv(sockfd, buf+total, sizeof buf - bytesleft, 0);
			printf("recv()'d %d bytes of data in buf\n", n);
			if (n == -1) { break; }
			if (n == 0) {
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
			if (f) {
				break;
			}
		}

		FILE *fptr;
		fptr = fopen("musicas", "r");
		fgets(bufout, 1024, fptr);
		if (buf[0] == '7') {
			
		}
		fclose(fptr);	
	}
	

	
	return 0;
}

