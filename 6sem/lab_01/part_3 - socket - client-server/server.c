#include <sys/types.h> 
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>

#define SOCKET "server.socket"
#define BUFFER_SIZE 64

static int sock_fd;

void sigint_handler() 
{
    close(sock_fd);
    unlink(SOCKET);
    exit(0);
}

int main(void)
{
	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd == -1)
    {
        perror("cant socket");
        exit(1);
    }
	struct sockaddr addr;
	addr.sa_family = AF_UNIX;
	strcpy(addr.sa_data, SOCKET);
	
	if (bind(sock_fd, &addr, sizeof(addr)) == -1) 
    {
	    perror("cant bind");
        exit(1);
    }
	struct sockaddr client_addr;
	int addrlen = sizeof(client_addr);
	
	if (signal(SIGINT, sigint_handler) == (void *)-1)
    {
        perror("cant signal");
        exit(1);
    }    

	char buf[BUFFER_SIZE];
    char copy_buf[BUFFER_SIZE];

    int bytes_read;
	while(1)
	{
        if ((bytes_read = recvfrom(sock_fd, buf, BUFFER_SIZE, 0, &client_addr, &addrlen)) == -1)
            perror("Can't recvfrom()");
        printf("recv message: %s\n", buf);
        
        buf[bytes_read] = '\0';
        sleep(2);

        sprintf(copy_buf, "%d: %s\n", getpid(), buf);
        printf("send: %s\n", copy_buf);
        if (sendto(sock_fd, copy_buf, strlen(copy_buf) + 1, 0, &client_addr, addrlen) == -1)
            perror("Can't sendto()");
	}
	
	return 0;
}