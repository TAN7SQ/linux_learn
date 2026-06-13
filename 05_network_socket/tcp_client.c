// SPDX-License-Identifier: GPL-2.0

#include <arpa/inet.h>
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
    int fd;
    int port;
    struct sockaddr_in addr;
    char buffer[256];
    ssize_t n;

    if (argc != 4) {
        fprintf(stderr, "usage: %s <server-ip> <port> <message>\n", argv[0]);
        return 1;
    }

    port = parse_port(argv[2]);
    if (port < 0) {
        fprintf(stderr, "invalid port: %s\n", argv[2]);
        return 1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1) {
        fprintf(stderr, "invalid server ip: %s\n", argv[1]);
        close(fd);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    if (write(fd, argv[3], strlen(argv[3])) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    memset(buffer, 0, sizeof(buffer));
    n = read(fd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("reply: %s", buffer);
    close(fd);
    return 0;
}
