#include "demo.h"
#include <errno.h>

/* Unix-like */
void errhandler_unixtype(char *msg);

/* Posix-like */
void errhandler_posixtype(int code, char *msg);

/* User-defined */
void errhandler_app(char *msg);
