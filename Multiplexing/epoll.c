#include "socklib.h"

#define MAXEVENTS 10

int main(int argc, char *argv[]) {
    char client_hostname[MAXLINE], client_portname[MAXLINE];
    char buf[MAXLINE];
    struct epoll_event ev, events[MAXEVENTS];
    struct sockaddr_storage clientaddr;
    int epollfd, listenfd, connfd, nfds;
    socklen_t clientlen;
    
    if (argc != 2) {
        exit(1);
    }
    
    epollfd = epoll_create1(0);
    
    listenfd = open_listenfd(argv[1]);
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }
    
    while (1) {
        nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(1);
        }
        for (int i = 0; i < nfds; i++) {
            int fd1 = events[i].data.fd;
            printf("FD %d events.\n", fd1);
            if (fd1 == listenfd) {
                clientlen = sizeof(clientaddr);
                connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
                getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_hostname, MAXLINE, 0);
                printf("Connected with (%s, %s) -> FD %d.\n", connfd, client_hostname, client_portname);
                ev.events = EPOLLIN | EPOLLHUP;
                ev.data.fd = connfd;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            } else {
                if (events[i].events & EPOLLIN) {
                    read(fd1, buf, MAXLINE);
                    dprintf(STDOUT_FILENO, "%s", buf);
                    dprintf(fd1, "Receive %d bytes.\n", strlen(buf));
                } else if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd1, NULL);
                    close(fd1);
                    dprintf(STDOUT_FILENO, "Disconnected with FD %d\n", fd1);
                }
            }
        }
    }
	return 0;
}

