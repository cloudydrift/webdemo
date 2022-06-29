#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <netdb.h>

#define LISTENQ 1024
#define MAXBUF 8192
#define MAXLINE 8192

typedef struct sockaddr SA;

typedef struct 
{
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[MAXBUF];
} rio_t;

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);

void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t n);

