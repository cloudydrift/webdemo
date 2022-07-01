#include "socklib.h"
#include "sbuf.h"

#define NTHREADS 4
#define SBUFSIZE 16

static int byte_cnt;
static sem_t mutex;

sbuf_t sbuf;

void echo_cnt(int connfd);
void *thread(void *vargp);

int main(int argc, char *argv[]) {
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    int i, listenfd, connfd;
    pthread_t tid;

    listenfd = open_listenfd(argv[1]);
    
    sbuf_init(&sbuf, SBUFSIZE);
    for (i = 0; i < NTHREADS; i++) {
        pthread_create(&tid, NULL, thread, NULL);
    }
    
    while (1) {
        clientlen = sizeof(clientaddr);
        // wait
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        // wait
        sbuf_insert(&sbuf, connfd);
    }

	return 0;
}

void *thread(void *vargp) {
    pthread_detach(pthread_self());
    while (1) {
        // wait
        int connfd = sbuf_remove(&sbuf);
        echo_cnt(connfd);
        close(connfd);
    }
}

static void init_echo_cnt(void) {
    sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void echo_cnt(int connfd) {
    int n;
    char buf[MAXLINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    
    pthread_once(&once, init_echo_cnt);
    rio_readinitb(&rio, connfd);
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        sem_wait(&mutex);
        byte_cnt += n;
        printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
        sem_post(&mutex);
        rio_writen(connfd, buf, n);
    } 
    
}
