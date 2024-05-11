#include "unp.h"

str_cli(FILE *fp, int sock_fd) {
    char sendline[MAXLINE], recvline[MAXLINE];
    while (Fgets(sendline, MAXLINE, fp) != NULL) {
        Writen(sock_fd, sendline, strlen (sendline));
        if (Readline(sock_fd, recvline, MAXLINE) == 0)
        rr_quit("str_cli: server terminated prematurely");
        Fputs(recvline, stdout);
    }
}

int main(int argc, char **argv) {
    int sock_fd;
    struct sockaddr_in servaddr;
    if (argc != 2)
        err_quit("usage: tcpcli <IPaddress>");
    sock_fd = Socket(PF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    Connect(sock_fd, (SA *) &servaddr, sizeof(servaddr));
    str_cli(stdin, sock_fd); /* do it all */
    exit(0);
}