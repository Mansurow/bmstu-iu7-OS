#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCK_NAME "socket.socket"
#define BUFFER_SIZE 64

int main()
{
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("cant socket");\
        exit(1);
    }

    struct sockaddr sockaddr = {.sa_family=AF_UNIX};
    strcpy(sockaddr.sa_data, SOCK_NAME);

    char buf[BUFFER_SIZE];
    sprintf(buf, "%d", getpid());
    printf("send: %s\n", buf);
    if (sendto(fd, buf, strlen(buf) + 1, 0, &sockaddr, sizeof(sockaddr)) == -1)
    {
        perror("cant sendto");
        exit(1);
    }
    close(fd);

    return 0;
}