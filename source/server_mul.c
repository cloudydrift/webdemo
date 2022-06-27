#include "riolib.h"
#include "socketlib.h"
#include <sys/select.h>

void echo(int connfd);
void command(void);

int main(int argc, char *argv[]) {
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
    fd_set read_set, ready_set;
	char client_hostname[MAXLINE], client_port[MAXLINE];

	if (argc != 2) {
		fprintf(stderr, "usage %s <port>\n", argv[0]);
		exit(0);
	}
	
	listenfd = open_listenfd(argv[1]);
	
	FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(listenfd, &read_set);

	while (1) {
        ready_set = read_set;
        select(listenfd+1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set))
            command();
        if (FD_ISSET(listenfd, &ready_set)) {
            clientlen = sizeof(clientaddr);
            connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
            getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
            printf("Connected with (%s, %s)", client_hostname, client_port);
            echo(connfd);
            close(connfd);
        }
	}
}

void echo(int connfd) {
	size_t n;
	char buf[MAXLINE];
	rio_t rio;

	rio_readinitb(&rio, connfd);

	while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
		printf("%s", buf);
        sprintf(buf, "Server received %d bytes.\n", (int)n);
        n = strlen(buf);
		rio_writen(connfd, buf, n+1);
	}
}

void command() {
    char buf[MAXLINE];
    if (!fgets(buf, MAXLINE, stdin))
        exit(0);
    printf("%s", buf);
}
