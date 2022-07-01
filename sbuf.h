#include <semaphore.h>
#include "demo.h"

typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;

/* Create an empty, bounded, shared FIFO buffer with a slots */
void sbuf_init(sbuf_t *sp, int n);

/* Clean up buffer sp */
void sbuf_deinit(sbuf_t *sp);

/* Insert Item onto the rear of shared buffer sp */
void sbuf_insert(sbuf_t *sp, int item);

/* Remove and return the first item from buffer sp */
int sbuf_remove(sbuf_t *sp);
