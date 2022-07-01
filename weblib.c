#include "weblib.h"

int weblib_clientconnect(char *hostname, char *port)
{
	int clientfd, rc;
	struct addrinfo hints, *listp, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }
    
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
		return -1;    
    
	else
		return clientfd;  
}

int weblib_serverlisten(char *port, int backlog)
{
	int listenfd, rc, optval = 1;
	struct addrinfo hints, *listp, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

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
    
	if (listen(listenfd, backlog) < 0) {
		close(listenfd);
		return -1;
	}
	
	return listenfd;
}
