#include "socklib.h"

#define MAXEVENTS 10

int main(int argc, char *argv[]) {
    struct sockaddr_storage clientaddr;
    struct epoll_event ev, events[MAXEVENTS];
    socklen_t clientlen = sizeof(clientaddr);
	int listenfd, connfd, epollfd, nfds;
    char buf[MAXLINE];
	
	if (argc != 2) {
		fprintf(stderr, "usage %s <port>\n", argv[0]);
		exit(0);
	}
    
    epollfd = epoll_create1(0);
    
    listenfd = open_listenfd(argv[1]);
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);

	while (1) {
        nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
                ev.events = EPOLLIN | EPOLLRDHUP;
                ev.data.fd = connfd;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            } else {
                if (events[i].events & (EPOLLRDHUP | EPOLLERR)) {
                    close(events[i].data.fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, NULL);
                } else {
                    read(events[i].data.fd, buf, MAXLINE);
                    printf("%s\n", buf);
                }
            }
        }
	}
	return 0;
}

