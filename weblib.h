#include <sys/socket.h>
#include <netdb.h>
#include "demo.h"

#define LISTENQ 1024

typedef struct sockaddr SA;

int weblib_clientconnect(char *hostname, char *port);
int weblib_serverlisten(char *port, int backlog);
