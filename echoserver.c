#include "riolib.h"
#include "socketlib.h"

int main(int argc, char *argv[]) {
	int listenfd, connfd;
	socketlen_t clientlen;
	struct socketaddr_storage clientaddr;
	char client_hostname[MAXLINE], client_port[MAXLINE];

	if (argc != 2) {
		fprintf(stderr, "usage %s <port>\n", argv[0]);
		exit(0);
	}

	listenfd = open_listenfd(argv[1]);

	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
		getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
		printf("Connected to (%s, %s)\n", client_hostname, client_port);
		echo(connfd);
		close(connfd);
	}

	return 0;
}

inline void echo(int connfd) {
	size_t n;
	char buf[MAXLINE];
	rio_t rio;

	rio_readinitb(&rio, connfd);

	while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
		printf("server received %d bytes\n", (int)n);
		rio_written(connfd, buf, n);
	}
}
