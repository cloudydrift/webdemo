#include "socklib.h"

#define MAXEVENTS 10

int main(int argc, char *argv[]) {
    char server_port[] = "12000";
    
    char httpbuf[MAXBUF];
    char client_hostname[MAXLINE], client_portname[MAXLINE];
    struct epoll_event ev;
    struct epoll_event *events;
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    int epollfd, listenfd, connfd, nfds;
    
    // init epoll instance
    epollfd = epoll_create1(0);
    
    // add listenfd
    /* input: listenfd epollfd; output: status*/
    listenfd = open_listenfd(server_port);
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }
    
    while (1) {
        // wait
        nfds = epoll_wait(epollfd, events, MAXEVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(1);
        }
        
        // foreach
        for (int i = 0; i < nfds; i++) {
            int fd1 = events[i].data.fd;
            // for listenfd
            if (fd1 == listenfd) {
                clientlen = sizeof(clientaddr);
                connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
                ev.events = EPOLLIN | EPOLLRDHUP;
                ev.data.fd = connfd;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            } 
            // for connfd
            else {
                if (events[i].events & EPOLLIN) {
                    int rn = rio_readn(fd1, httpbuf, MAXBUF);
                    if (rn < 0)
                        perror("rio_readn");
                    httpbuf[rn] = '\0';
                    printf("%s", httpbuf);
                    sprintf(httpbuf, "Received %d bytes.\n", rn);
                    int wn = rio_writen(fd1, httpbuf, rn);
                    if (wn < 0)
                        perror("rio_writen");
                } 
                if (events[i].events & EPOLLRDHUP) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd1, NULL);
                    close(fd1);
                    printf("Disconnection: %d.\n", fd1);
                }
                if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                    printf("HUP | ERR, Can this occur?\n");
                }
            }
        }
    }
	return 0;
}

