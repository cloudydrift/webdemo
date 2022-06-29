#include "riolib.h"
#include "socketlib.h"
#include <sys/select.h>

void echo(int connfd);
void command(void);
void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);

typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
} pool;

int byte_cnt = 0;

int main(int argc, char *argv[]) {
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
    fd_set read_set, ready_set;
	char client_hostname[MAXLINE], client_port[MAXLINE];
    
    static pool pool;
    
	if (argc != 2) {
		fprintf(stderr, "usage %s <port>\n", argv[0]);
		exit(0);
	}
	
	listenfd = open_listenfd(argv[1]);
    init_pool(listenfd, &pool);

	while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);
    
        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(clientaddr);
            connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
//             getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
//             printf("Connected with (%s, %s).\n", client_hostname, client_port);
            add_client(connfd, &pool);
        }
        check_clients(&pool);
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
		rio_writen(connfd, buf, n);
	}
}

void init_pool(int listenfd, pool *p) {
    int i;
    p->maxi = 01;
    for (i=0; i<FD_SETSIZE; i++)
        p->clientfd[i] = -1;
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p) {
    int i;
    p->nready--;
    for (i=0; i<FD_SIZE; i++)
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            rio_readinitb(&p->clientrio[i], connfd);
            
            FD_SET(connfd, &p->read_set);
            
            if (connfd > p->maxfd)
                p->maxfd = connfd;
            if (i > p->maxi)
                p->maxi = i;
            break;
        }
    if (i == FD_SIZE) {
        printf("too many clients.\n");
        exit(0);
    }
}
