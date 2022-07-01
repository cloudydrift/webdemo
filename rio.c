#include "rio.h"

void rio_initbuf(riobuf_t *rbp, int fd)
{
    rbp->rio_fd = fd;
    rbp->rio_cnt = 0;
    rbp->rio_bufp = rbp->rio_buf;
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        nread = read(fd, bufp, nleft);
        if (nread < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0)
            break;
        nleft -= nread;
        bufp += nread;
    }
    
    return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwrite;
    char *bufp = usrbuf;

    while (nleft > 0) {
        nwrite = write(fd, bufp, nleft);
        if (nwrite < 0) {
            if (errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        bufp += nwrite;
    }
    
    return n;
}

/* a static function to I/O with a rioBuf buffer */
static ssize_t rio_read(riobuf_t *rbp, void *usrbuf, size_t n)
{
    int cnt;

    while (rbp->rio_cnt <= 0) {
        rbp->rio_cnt = read(rbp->rio_fd, rbp->rio_buf, sizeof(rbp->rio_buf));
        if (rbp->rio_cnt < 0) {
            if (errno != EINTR)
                return -1;
        } else if (rbp->rio_cnt == 0)
            return 0;
        else
            rbp->rio_bufp = rbp->rio_buf;
    }
    
    cnt = min(n, rbp->rio_cnt);
    memcpy(usrbuf, rbp->rio_bufp, cnt);
    rbp->rio_bufp += cnt;
    rbp->rio_cnt -= cnt;
    
    return cnt;
}

ssize_t rio_bufreadn(riobuf_t *rbp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
        nread = rio_read(rbp, bufp, nleft);
        if (nread < 0)
            return -1;
        else if (nread == 0) 
            break;
        nleft -= nread;
        bufp += nread;
    }
    
    return (n - nleft);
}

ssize_t rio_bufreadline(riobuf_t *rbp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    
    for (n = 0; n < maxlen - 1; n++) {
        rc = rio_read(rbp, &c, 1);
        if (rc == 1) {
            *bufp++ = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 0)
                return 0;
            else
                break;
        } else
            return -1;
    }
    *bufp = 0;

    return n;
}
