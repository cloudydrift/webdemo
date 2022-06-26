/* robust I/O lib */
#include <sys/stat.h>
#include <dirent.h>
#include "netdemo.h"

#define MAXBUF 8192
#define MAXLINE 8192

typedef struct 
{
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[MAXBUF];
} rio_t;

void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t n);
