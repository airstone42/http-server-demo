#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define BUF_MAX 40960
#define SHORT_MAX 128
#define OPEN_MAX 1024
#define PORT 8080

extern const char default_html[];

enum http_method {
    GET,
    POST,
    OTHERS,
};

struct request {
    enum http_method method;
    char host[SHORT_MAX];
};

struct handle_args {
    int sock_fd;
    char *buf;
    size_t buf_len;
};

void handle(struct handle_args *args);

#endif
