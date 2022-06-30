#include "socklib.h"

#define EPOLLPOLLSIZE 20

struct ConnfdNode {
    int connfd;
    rio_t *rio;
};

typedef struct {
    int epollfd;
    int listenfd;
    int numConnfds;
    struct ConnfdNode connfds[EPOLLPOLLSIZE];
    rio_t rios[EPOLLPOLLSIZE];
    void (*epollInit)(void) ;
} EpollPool;

void epollInit(void) {
    memset();
}
