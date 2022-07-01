#include "weblib.h"
#include "rio.h"
#include "errhandler.h"

void doit(int fd);
void read_requesthdrs(riobuf_t *rbp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

void sigchld_handler(int signum);

extern char **environ;

int main(int argc, char **argv) 
{
    pid_t pid;
    int listenfd, connfd;
    char clienthostname[MAXLINE], clientport[MAXLINE];
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    
    if (argc != 2)
        errhandler_app("<port>");

    listenfd = weblib_serverlisten(argv[1], LISTENQ);
    signal(SIGCHLD, sigchld_handler);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (connfd < 0)
            errhandler_unixtype("accept");
        getnameinfo((SA *) &clientaddr, clientlen, clienthostname, MAXLINE, clientport, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", clienthostname, clientport);
        
        pid = fork();
        if (pid == -1)
            errhandler_unixtype("fork");
        else if (pid == 0) {
            close(listenfd);
            doit(connfd);                                     
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
    return 0;
}

void sigchld_handler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
    return;
}

void doit(int fd) 
{
    int is_static;
    struct stat filestat;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    riobuf_t riobuf;

    /* Read request line and headers */
    rio_initbuf(&riobuf, fd);
    if (!rio_bufreadline(&riobuf, buf, MAXLINE))  
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);      
    if (strcasecmp(method, "GET")) {         
        clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
        return;
    }                                                 
    read_requesthdrs(&riobuf);                           

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);     
    if (stat(filename, &filestat) < 0) {            
        clienterror(fd, filename, "404", "Not found","Tiny couldn't find this file");
        return;
    }                                          

    if (is_static) { /* Serve static content */          
        if (!(S_ISREG(filestat.st_mode)) || !(S_IRUSR & filestat.st_mode)) { 
            clienterror(fd, filename, "403", "Forbidden","Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, filestat.st_size);    
    } else { /* Serve dynamic content */
        if (!(S_ISREG(filestat.st_mode)) || !(S_IXUSR & filestat.st_mode)) { 
            clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);        
    }
}

void read_requesthdrs(riobuf_t *rbp) 
{
    char buf[MAXLINE];

    rio_bufreadline(rbp, buf, MAXLINE);
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) {   
        rio_bufreadline(rbp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */ 
        strcpy(cgiargs, "");                      
        strcpy(filename, ".");                      
        strcat(filename, uri);                      
        if (uri[strlen(uri)-1] == '/')            
            strcat(filename, "home.html");             
        return 1;
    } else {  /* Dynamic content */          
        ptr = index(uri, '?');            
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        } else 
            strcpy(cgiargs, "");                    
        strcpy(filename, ".");                     
        strcat(filename, uri);                         
        return 0;
    }
}

void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);   
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    rio_writen(fd, buf, strlen(buf));  

    /* Send response body to client */
    srcfd = open(filename, O_RDONLY, 0); 
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); 
    close(srcfd);                    
    rio_writen(fd, srcp, filesize); 
    munmap(srcp, filesize);            
}

void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
} 

void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    rio_writen(fd, buf, strlen(buf));
  
    if (fork() == 0) { /* Child */ 
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1); 
        dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ 
        execve(filename, emptylist, environ); /* Run CGI program */
    }
    wait(NULL); /* Parent waits for and reaps child */
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    rio_writen(fd, buf, strlen(buf));
}
