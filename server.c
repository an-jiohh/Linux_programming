#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>
#include <time.h>
#include "book.c"

#define BUFFER_SIZE 256

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void controller(int socketFd, char *argv[])
{
    int memberFd = open(argv[3], O_RDWR);
    if (memberFd == -1)
    { // 파일이 없을경우
        memberFd = open(argv[3], O_RDWR | O_CREAT, 0660);
        int start = 1;
        int check = write(memberFd, &start, sizeof(int));
        member temp;
        write(memberFd, (char *)&temp, sizeof(temp));
    }
    int bookFd = open(argv[2], O_RDWR);
    if (bookFd == -1)
    { // 파일이 없을경우
        bookFd = open(argv[2], O_RDWR | O_CREAT, 0660);
        int start = 0;
        int check = write(bookFd, &start, sizeof(int));
    }
    while (1)
    {
        member record;
        record = loginMenu(socketFd, memberFd);
        if (record.key != NO_DATA)
        {
            bookMenu(socketFd, bookFd, memberFd, record);
        }
        if (record.key == -300)
            break;
    }
    close(memberFd);
    close(bookFd);
}

void sock(char *argv[])
{

    int sockfd, newsockfd, portno, clilen, pid;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opeing socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)
        {
            close(sockfd);
            controller(newsockfd, argv);
            exit(0);
        }
        else
            close(newsockfd);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sock(argv);

    return 0;
}