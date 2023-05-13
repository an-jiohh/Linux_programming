#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void childEnd()
{
    exit(0);
}

void controller(int sockfd)
{
    int pid = fork();

    if (pid == 0)
    {
        char in_buffer[BUFFER_SIZE];
        while (1)
        {
            bzero(in_buffer, sizeof(in_buffer));
            read(sockfd, in_buffer, sizeof(in_buffer));
            printf("%s", in_buffer);
            fflush(stdout);
            if (strcmp(in_buffer, "end") == 0)
            {
                close(sockfd);
                exit(0);
            }
        }
    }
    else
    {
        char out_buffer[BUFFER_SIZE];
        while (1)
        {
            bzero(out_buffer, sizeof(out_buffer));
            scanf(" %s", out_buffer);
            write(sockfd, out_buffer, sizeof(out_buffer));
        }
    }
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD, childEnd);
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3)
    {
        fprintf(stderr, "ERROR, file hostname port\n");
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opeing socket");

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    controller(sockfd);

    return 0;
}