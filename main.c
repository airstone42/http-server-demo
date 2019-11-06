#include "server.h"

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    int sock_fd = 0;

    struct sockaddr_in srv_addr;
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(PORT);
    int sock_opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));

    if (bind(listen_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("Failed to listen to socket");
        exit(EXIT_FAILURE);
    }

    char buf[BUF_MAX];
    bzero(&buf, sizeof(buf));
    struct epoll_event ev;
    struct epoll_event events[OPEN_MAX];
    bzero(&ev, sizeof(ev));
    bzero(events, sizeof(events));

    int epoll_fd = 0;
    if ((epoll_fd = epoll_create(1)) < 0) {
        perror("Failed to create epoll instance");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    if ((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0)) {
        perror("Failed to add epoll event");
        exit(EXIT_FAILURE);
    }
    int cap = 0;
    int ready = 0;

    const char *split_string = "\r\n\r\n";

    while (true) {
        if ((ready = epoll_wait(epoll_fd, events, cap + 1, -1)) < 0) {
            perror("Failed to poll fds");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < ready; i++) {
            if (events[i].data.fd == listen_fd && (events[i].events & EPOLLIN)) {
                struct sockaddr_in cli_addr;
                socklen_t cli_addr_size = sizeof(cli_addr);
                int conn_fd = accept(listen_fd, (struct sockaddr *) &cli_addr, &cli_addr_size);
                if (conn_fd < 0) {
                    perror("Failed to accept connection");
                    exit(EXIT_FAILURE);
                }

                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                if ((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0)) {
                    perror("Failed to add epoll event");
                    exit(EXIT_FAILURE);
                }
                cap++;
                continue;
            } else if (events[i].events & EPOLLIN) {
                ssize_t n;
                sock_fd = events[i].data.fd;

                if ((n = read(sock_fd, buf, sizeof(buf))) <= 0) {
                    if (close(sock_fd) < 0) {
                        perror("Failed to close socket");
                        exit(EXIT_FAILURE);
                    }
                    continue;
                } else {
                    size_t size = strlen(buf);
                    if (memcmp(buf + size - strlen(split_string), split_string, strlen(split_string)) == 0) {
                        pthread_t thread;
                        struct handle_args *args = (struct handle_args *) malloc(sizeof(struct handle_args));
                        char *new_buf = (char *) malloc(sizeof(buf));
                        strncpy(new_buf, buf, sizeof(buf));
                        args->sock_fd = sock_fd;
                        args->buf = new_buf;
                        args->buf_len = strlen(buf);

                        pthread_create(&thread, NULL, (void *(*)(void *)) handle, (void *) args);
                        write(fileno(stdout), buf, n);
                    }
                }
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &events[i]);
                cap--;
                close(events[i].data.fd);
            }
        }
    }
    close(listen_fd);
    return 0;
}
