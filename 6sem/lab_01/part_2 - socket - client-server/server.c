#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#define SOCK_NAME "socket.socket"
#define BUFFER_SIZE 64

int fd;

void sigint_handler()
{
    close(fd);
    unlink(SOCK_NAME);
    printf("Server shutdown...\n");
    exit(0);
}

int main() 
{
    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("cant socket");
        exit(1);
    }
    struct sockaddr sockaddr = {.sa_family=AF_UNIX};
    strcpy(sockaddr.sa_data, SOCK_NAME);

    if (bind(fd, &sockaddr, sizeof(sockaddr)) == -1)
    {
        perror("cant bind");
        exit(1);
    }

    if (signal(SIGINT, sigint_handler) == (void *)-1)
    {
        perror("cant signal");
        exit(1);
    }

    char buf[BUFFER_SIZE];
    while(1)
    {
        int bytes_read = read(fd, buf, BUFFER_SIZE);

        if (bytes_read == -1) 
        {
            perror("Can't read()");
            exit(1);
        }    
        else
        {
            buf[bytes_read] = '\0';
            printf("read message: %s\n", buf);
        }

        sleep(1);
    }

    return 0;
}

