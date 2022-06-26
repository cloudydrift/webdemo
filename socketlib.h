#include <sys/socket.h>
#include <netdb.h>
#include "netdemo.h"

#define LISTENQ 1024

typedef struct sockaddr SA;

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);