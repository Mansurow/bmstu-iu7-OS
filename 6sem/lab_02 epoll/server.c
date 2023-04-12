#define _GNU_SOURCE

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_EVENTS 4
#define BUFF_SIZE 32

int main(void)
{
    int port = 5000;
	
    struct epoll_event events[MAX_EVENTS];
    
    int conn_sock, nfds;

    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        perror("can't sock");
        exit(EXIT_FAILURE);
    }

    if (fcntl(listen_sock, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("can't listen");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_addr = {
            .s_addr = INADDR_ANY
        },
        .sin_port = htons(port)
    };

    if (bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        perror("can't bind");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_sock, MAX_EVENTS) == -1) 
    {
        perror("can't listen");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev; 
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    printf("server_pid: %d\n", getpid());
    
    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            close(listen_sock);
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                struct sockaddr client_addr;
                socklen_t client_len;
                conn_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_len);

                if (conn_sock == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    close(listen_sock);
                    exit(EXIT_FAILURE);
                }
                
            } else {
                int conn = events[i].data.fd;

                char msg_to[BUFF_SIZE], msg_from[BUFF_SIZE];

                msg_to[BUFF_SIZE - 1] = 0;
                msg_from[BUFF_SIZE - 1] = 0;

                int rc = read(conn, msg_from, sizeof(msg_from));

                if (rc == -1) {
                    perror("can't read");
                    close(conn);
                    exit(EXIT_FAILURE);
                }
                
                if (rc > 0) 
                {
                    printf("\nreceived client_pid: %s\n", msg_from);
                    sprintf(msg_to, "%d_%s", getpid(), msg_from);

                    if (send(conn, msg_to, sizeof(msg_to), 0) == -1) {
                        perror("can't send");
                        exit(EXIT_FAILURE);
                    }

                    printf("send to client: %s\n", msg_to);
                }
                close(conn);
            }
        }
    }
}