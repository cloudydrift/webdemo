#include "socklib.h"

int open_clientfd(char *hostname, char *port) {
	int clientfd;
	struct addrinfo hints, *listp, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
	getaddrinfo(hostname, port, &hints, &listp);

	for (p = listp; p; p = p->ai_next) {
		clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (clientfd < 0)
			continue;
		if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
			break;
		close(clientfd);
	}
	
	freeaddrinfo(listp);

	if (!p)
		return -1;        // no host or connection falls
	else
		return clientfd;  // conection passes
}

int open_listenfd(char *port) {
	int listenfd, optval = 1;
	struct addrinfo hints, *listp, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
	getaddrinfo(NULL, port, &hints, &listp);

	for (p = listp; p; p = p->ai_next) {
		listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listenfd < 0)
			continue;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval));
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
		close(listenfd);
	}

	freeaddrinfo(listp);

	if (!p)
		return -1;
	if (listen(listenfd, LISTENQ) < 0) {
		close(listenfd);
		return -1;
	}
	return listenfd;
}

void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        nread = read(fd, bufp, nleft);
        if (nread < 0)
        {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)   // EOF
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        nwritten = write(fd, bufp, nleft);
        if (nwritten < 0)
        {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

static ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0)    
    {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0)
        {
            if (errno != EINTR)
                return -1;
        }
        else if (rp->rio_cnt == 0)   // EOF
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf;
    }
    cnt = n;
    if (rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0)
    {
        nread = rio_read(rp, bufp, nleft);
        if (nread < 0)
            return -1;
        else if (nread == 0)   // EOF
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    
    for (n = 0; n < maxlen - 1; n++)
    {
        rc = rio_read(rp, &c, 1);
        if (rc == 1)
        {
            *bufp++ = c;
            if (c == '\n')
            {
                n++;
                break;
            }
        }
        else if (rc == 0)   // EOF
        {
            if (n == 0)
                return 0;
            else
                break;
        }
        else
            return -1;
    }
    *bufp = 0;     // NULL
    return n;
}
