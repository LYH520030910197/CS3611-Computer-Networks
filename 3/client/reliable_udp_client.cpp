/*
** reliable_udp_client.c -- a udp socket client requesting files through reliable transmissions
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

#define PORT "4951"
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
int open_clientfd(const char *hostname, const char *port, struct sockaddr *addr, socklen_t *addrlen)
{
    int clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_DGRAM;  /* Open a connection */
    hints.ai_family = AF_INET;       /* Use IPv4 */
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

    *addr = *p->ai_addr;
    *addrlen = p->ai_addrlen;
    return clientfd;
}

struct packet
{
    size_t sn;
    char buf[MAXLINE];
};

size_t sn = 0;
/*
 * reliable_recv - receive the packet and send ACK packet to the sender
 */
ssize_t reliable_recv(int sockfd, char *buf, size_t len, int flags,
                      struct sockaddr *src_addr, socklen_t *addrlen)
{
    struct packet packet;
    ssize_t size;
    while ((size = recv(sockfd, &packet, sizeof(packet), 0)))
    {
        if (size != -1 && packet.sn == sn)
        {
            /* Normal packet */
            for (ssize_t i = 0; i < size - sizeof(size_t); ++i)
                buf[i] = packet.buf[i];
            sendto(sockfd, &packet, sizeof(size_t), 0, src_addr, *addrlen);
            ++sn;
            return size - sizeof(size_t);
        }
        else if (sn == 0)
        {
            /* The first packet lost, retransmit the filename */
            return -1;
        }
        else
        {
            /* Some other packet lost, request retransmission */
            packet.sn = sn - 1;
            sendto(sockfd, &packet, sizeof(size_t), 0, src_addr, *addrlen);
        }
    }
}

/*
 * recv_file - send the filename, and receive the file from the server
 */
void recv_file(int connfd, const char *filename, struct sockaddr *addr, socklen_t *addrlen)
{
    char buf[MAXLINE];
    ssize_t size;

    /* Set timeout interval to 100ms */
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    FILE *file = fopen(filename, "wb");
    time_t start;
    do {
        /* Send filename */
        sendto(connfd, filename, strlen(filename), 0, addr, *addrlen);

        /* Receive file contents */
        sn = 0;
        start = time(NULL);
        while ((size = reliable_recv(connfd, buf, MAXLINE, 0, addr, addrlen)) > 0)
            fwrite(buf, 1, size, file);
    } while (sn == 0);
    fclose(file);
    printf("%s received, %ld seconds for transmission\n", filename, time(NULL) - start);
}

/*
 * main - Main routine for the file client
 */
int main(int argc, char **argv)
{
    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server address> <filename>\n", argv[0]);
        return 0;
    }

    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_storage);
    int clientfd = open_clientfd(argv[1], PORT, (struct sockaddr *)&clientaddr, &clientlen);
    recv_file(clientfd, argv[2], (struct sockaddr *)&clientaddr, &clientlen);
    close(clientfd);
    return 0;
}
