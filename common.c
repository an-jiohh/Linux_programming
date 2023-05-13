#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>
#include <stdarg.h>
#include <fcntl.h>
#include "common.h"

#define NO_DATA -100
#define BUFFER_SIZE 256

void iprintf(int sockfd, char *ch, ...)
{
    char out_buffer[BUFFER_SIZE];
    bzero(out_buffer, sizeof(out_buffer));

    va_list arg;
    int done;

    va_start(arg, ch);
    vsprintf(out_buffer, ch, arg);
    va_end(arg);

    write(sockfd, out_buffer, sizeof(out_buffer));
}

void iscanf(int sockfd, char *ch)
{
    char in_buffer[BUFFER_SIZE];
    bzero(in_buffer, sizeof(in_buffer));
    read(sockfd, in_buffer, 256);

    strcpy(ch, in_buffer);
}

void iscanfInt(int sockfd, int *ch)
{
    char in_buffer[BUFFER_SIZE];
    bzero(in_buffer, sizeof(in_buffer));
    read(sockfd, in_buffer, 256);
    *ch = atoi(in_buffer);
}

void nowtime(int socketFd)
{
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    printf("[%02d:%02d:%02d]", now->tm_hour, now->tm_min, now->tm_sec);
}