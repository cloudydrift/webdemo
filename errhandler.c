#include "errhandler.h"

void errhandler_unixtype(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void errhandler_posixtype(int code, char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(code));
    exit(0);
}

void errhandler_app(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}
