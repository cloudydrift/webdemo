#include "socklib.h"

#define MAXEVENTS 10

void echo(int fd);

int main(int argc, char *argv[]) {
    struct sockaddr_storage clientaddr;
    struct epoll_event ev, events[MAXEVENTS];
    socklen_t clientlen = sizeof(clientaddr);
	int listenfd, connfd, epollfd, nfds;
	
	if (argc != 2) {
		fprintf(stderr, "usage %s <port>\n", argv[0]);
		exit(0);
	}
	
	listenfd = open_listenfd(argv[1]);
    
    epollfd = epoll_create(10);
    
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

	while (1) {
        nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
                ev.events = EPOLLIN;
                ev.data.fd = connfd;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            } else {
                echo(events[i].data.fd);
            }
        }
	}
	return 0;
}

void echo(int connfd) {
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    rio_readinitb(&rio, connfd);
    while((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int) n);
        rio_writen(connfd, buf, n);
    }
}

