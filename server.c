#include "server.h"

const char default_html[] =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"utf-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n"
        "    <title>Server Demo</title>\n"
        "</head>\n"
        "<body>\n"
        "    <header>\n"
        "        <h1>Default Index</a></h1>\n"
        "    </header>\n"
        "    <p>This is an default index page.</p>\n"
        "</body>\n"
        "</html>\n";

void handle(struct handle_args *args) {
    pthread_detach(pthread_self());
    args = (struct handle_args *) args;

    struct request req;
    bzero(&req, sizeof(req));
    if (strncmp(args->buf, "GET", 3) == 0)
        req.method = GET;
    else if (strncmp(args->buf, "POST", 4) == 0)
        req.method = POST;
    else
        req.method = OTHERS;

    char *host_string = "Host: ";
    char *newline_string = "\r\n";
    char *begin = strstr(args->buf, host_string);
    if (begin) {
        char *end = strstr(begin, newline_string);
        strncat(req.host, begin + strlen(host_string), end - begin - strlen(host_string));
    }

    char date[SHORT_MAX];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);

    size_t html_length = strlen(default_html);
    char content_length[3];
    bzero(content_length, sizeof(content_length));
    sprintf(content_length, "%lu", html_length);

    char send_buf[BUF_MAX];
    bzero(send_buf, sizeof(send_buf));
    strcat(send_buf, "HTTP/1.1 200 OK\r\n");
    strcat(send_buf, "Host: ");
    strcat(send_buf, req.host);
    strcat(send_buf, "\r\n");
    strcat(send_buf, "Date: ");
    strcat(send_buf, date);
    strcat(send_buf, "\r\n");
    strcat(send_buf, "Content-type: text/html; charset=UTF-8\r\n");
    strcat(send_buf, "Content-length: ");
    strcat(send_buf, content_length);
    strcat(send_buf, "\r\n\r\n");
    const char *html_buf = default_html;
    strcat(send_buf, html_buf);
    write(args->sock_fd, send_buf, strlen(send_buf));
    free(args->buf);
    free(args);
}
