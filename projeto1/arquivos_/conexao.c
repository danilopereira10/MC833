#include <unistd.h>

struct in_addr {
    uint32_t s_addr; //that's a 32-bit int (4 bytes)
}

struct sockaddr_in {
    short int sin_family; //Address family, AF_INET
    unsigned short int sin_port; //Port number
    struct in_addr sin_addr; //Internet address
    unsigned char sin_zero[8]; //Same size as struct sockaddr
}

int status
struct addrinfo hints;
struct addrinfo *servinfo // will point to the results

menset(&hints, 0, sizeof(hints)); // make sure the struct is empty
hints.ai_family = AF_UNSPEC; //don't care IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
hints.ai_flags = AI_PASSIVE // fill in my IP for me

if (( status = getaddrinfo (NULL, "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
}

getddrinfo("www.example.com", "http", &hints, &res)
s = socket (res->ai_family, res->ai_socktype, res->ai_protocol)

getaddrinfo(NULL, "3490", &hints, &res)
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)
if (bind (sockfd, res->ai_addr, res->ai_addrlen) < 0)
    perror(“bind call error”);

if (connect (sockfd, res->ai_addr, res->ai_addrlen)) < 0)
    perror(“connect call error”);

if (listen (sd, 2) < 0)
    perror (“listen call error”);


getaddrinfo(NULL, MYPORT, &hints, &res);
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
bind(sockfd, res->ai_addr, res->ai_addrlen);
listen(sockfd, BACKLOG);
// now accept an incoming connection:
addr_size = sizeof their_addr;
new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

pid_t fork(void);
    Returns: 0 in child, process ID of child in parent, -1 on error

pid_t pid;
int sock_fd, new_fd;
/* fill in sockaddr_in{ } with server's well-known port */
sock_fd = socket( ... );
bind(sock_fd, ... );
listen(sock_fd, LISTENQ);

for ( ; ; ) {
    new_fd = accept (sock_fd, ... ); /* probably blocks */

    if( (pid = fork() ) == 0) {
        close(sock_fd); /* child closes listening socket */
        doit(new_fd); /* process the request */
        close(new_fd); /* done with this client */
        exit(0); /* child terminates */
    }

    close(new_fd); /* parent closes connected socket */
}

