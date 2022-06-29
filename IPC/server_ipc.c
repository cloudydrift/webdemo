#include "socklib.h"

void echo(int connfd);
void sigchld_handler(int sig);

int main(int argc, char *argv[]) {
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	char client_hostname[MAXLINE], client_port[MAXLINE];

	if (argc != 2) {
		fprintf(stderr, "usage %s <port>\n", argv[0]);
		exit(0);
	}

    signal(SIGCHLD, sigchld_handler);
	listenfd = open_listenfd(argv[1]);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected with (%s, %s)\n", client_hostname, client_port);
        if (fork() == 0) {
            close(listenfd);
            echo(connfd);
            close(connfd);
            exit(0);
        }
		close(connfd);
	}

	return 0;
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
    return;
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
