/*
** tcp_client.c -- a tcp socket client requesting files
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
#include <time.h>

#define PORT "3490"
#define MAXLINE 8192

/*
 * open_clientfd - Open connection to server at <hostname, port> and
 *     return a socket descriptor ready for reading and writing. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for getaddrinfo error
 *       -1 with errno set for other errors.
 */
int open_clientfd(const char *hostname, const char *port)
{
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV; /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG; /* Recommended for connections */
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0)
    {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can successfully connect to */
    for (p = listp; p; p = p->ai_next)
    {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break; /* Success */
        if (close(clientfd) < 0)
        { /* Connect failed, try another */
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else /* The last connect succeeded */
        return clientfd;
}

/*
 * recv_file - send the filename, and receive the file from the server
 */
void recv_file(int connfd, const char *filename)
{
    char buf[MAXLINE];
    ssize_t size;

    /* Send filename */
    send(connfd, filename, strlen(filename), 0);

    /* Receive file contents */
    FILE *file = fopen(filename, "wb");
    time_t start = time(NULL);
    while ((size = recv(connfd, buf, MAXLINE, 0)) > 0)
        fwrite(buf, 1, size, file);
    fclose(file);
    printf("%s received, %ld seconds for transmission\n", filename, time(NULL) - start);
}

/*
 * main - Main routine for the file client
 */
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server address> <filename>\n", argv[0]);
        return 0;
    }

    int clientfd = open_clientfd(argv[1], PORT);
    recv_file(clientfd, argv[2]);
    close(clientfd);
    return 0;
}
