// SPDX-License-Identifier: GPL-2.0

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int parse_port(const char *text)
{
    char *end = NULL;
    long port = strtol(text, &end, 10);

    if (*text == '\0' || *end != '\0' || port <= 0 || port > 65535)
        return -1;
    return (int)port;
}

int main(int argc, char **argv)
{
    const char *bind_ip;
    int port;
    int server_fd;
    int client_fd;
    struct sockaddr_in addr;
    char buffer[256];
    ssize_t n;
    int opt = 1;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <bind-ip> <port>\n", argv[0]);
        return 1;
    }

    bind_ip = argv[1];
    port = parse_port(argv[2]);
    if (port < 0) {
        fprintf(stderr, "invalid port: %s\n", argv[2]);
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, bind_ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "invalid bind ip: %s\n", bind_ip);
        close(server_fd);
        return 1;
    }

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 4) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("tcp_server listening on %s:%d\n", bind_ip, port);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));
    n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    printf("rx: %s\n", buffer);
    if (write(client_fd, "orangepi:ok\n", strlen("orangepi:ok\n")) < 0)
        perror("write");

    close(client_fd);
    close(server_fd);
    return 0;
}
