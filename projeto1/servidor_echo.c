#include "unp.h"

void str_echo(int new_fd) {
    ssize_t n;
    char buf[MAXLINE];

    while (true) {
        while((n = read(new_fd, buf, MAXLINE)) > 0) {
            writen(new_fd, buf, n);
        }
        if (n < 0 && errno == EINTR) {
            continue;
        } else if (n < 0) {
            perror("str_echo: read error");
            break;
        }
        break;
    }
}

int main(int argc, char **argv) {
    int sock_fd, new_fd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    sock_fd= Socket (PF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (SERV_PORT);
    Bind(sock_fd, (SA *) &servaddr, sizeof(servaddr));
    Listen(sock_fd, LISTENQ);
    for ( ; ; ) {
        clilen = sizeof(cliaddr);
        new_fd= Accept(sock_fd, (SA *) &cliaddr, &clilen);
        if ( (childpid = Fork()) == 0) { /* child process */
            Close(sock_fd); /* close listening socket */
            str_echo(new_fd); /* process the request */
            exit (0);
        }
        Close(new_fd); /* parent closes connected socket */
    }
}