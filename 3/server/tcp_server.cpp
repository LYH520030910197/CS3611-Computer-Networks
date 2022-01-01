/*
** tcp_server.c -- a tcp socket server sending files
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT "3490"
#define MAXLINE 8192
#define LISTENQ 1024

/*  
 * open_listenfd - Open and return a listening socket on port. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for getaddrinfo error
 *       -1 with errno set for other errors.
 */
int open_listenfd(const char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval = 1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0)
    {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        if (close(listenfd) < 0)
        { /* Bind failed, try the next */
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}

/*
 * recv_file - receive the file name, and send the file to the client
 */
void send_file(int connfd)
{
    char buf[MAXLINE], filename[MAXLINE];
    ssize_t size;

    /* Receive filename */
    if ((size = recv(connfd, filename, MAXLINE - 1, 0)) > 0)
        filename[size] = '\0';
    else
        return;
    printf("%s requested\n", filename);
    
    /* Send file contents */
    FILE *file = fopen(filename, "rb");
    while ((size = fread(buf, 1, MAXLINE, file)) > 0)
        send(connfd, buf, size, 0);
    fclose(file);
}

/*
 * main - Main routine for the file server
 */
int main(int argc, char **argv)
{
    int listenfd = open_listenfd(PORT);
    while (1)
    {
        int connfd = accept(listenfd, NULL, NULL);
        send_file(connfd);
        close(connfd);
    }
    return 0;
}
