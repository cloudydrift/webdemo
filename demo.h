#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXBUF 8192
#define MAXLINE 8192

#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))
