/* a robust I/O lib */

#include "demo.h"
#include <errno.h>

typedef struct  {
    int rio_fd;
    int rio_cnt;
    char *rio_bufp;
    char rio_buf[MAXBUF];
} riobuf_t;

/* init a rio buffer */
void rio_initbuf(riobuf_t *rbp, int fd);

/* read n bytes without buffer */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);

/* write n bytes without buffer */
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

/* read n bytes with a rio buffer */
ssize_t rio_bufreadn(riobuf_t *rbp, void *usrbuf, size_t n);

/* read a line with a rio buffer */
ssize_t rio_bufreadline(riobuf_t *rbp, void *usrbuf, size_t n);
