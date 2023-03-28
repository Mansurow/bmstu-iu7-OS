#include <sys/types.h> 
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_SOCKET "server.socket"
#define BUFFER_SIZE 64

int main(void)
{
	int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if  (sockfd == -1)
	{
		perror("cant socket");
		exit(1);
	}
	
	struct sockaddr server_addr = {.sa_family = AF_UNIX};
	strcpy(server_addr.sa_data, SERVER_SOCKET);

	struct sockaddr sockaddr = {.sa_family = AF_UNIX};
	sprintf(sockaddr.sa_data, "%d.socket", getpid());
	
	if (bind(sockfd, &sockaddr, sizeof(sockaddr)) == -1)
	{
		perror("cant bind");
		exit(1);
	}	
	
	char buf[BUFFER_SIZE];
    sprintf(buf, "%d", getpid());

	if (sendto(sockfd, buf, strlen(buf), 0, &server_addr, sizeof(server_addr)) == -1)
	{
		perror("cant send");
		exit(1);
	}
    printf("send: %s\n", buf);

	int bytes_read;
    if ((bytes_read = read(sockfd, buf, BUFFER_SIZE)) == -1) 
	{
		perror("cant read");
		exit(1);
	}
	buf[bytes_read] = '\0';
	printf("read message: %s\n", buf);

	close(sockfd);
    unlink(sockaddr.sa_data);
	return 0;
}