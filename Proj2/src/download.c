#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#define MAX_LENGTH 500
#define PORT 21
#define TRANSFER_READY 150
#define AUTH_READY 220
#define LOGOUT 221
#define TRANSFER_COMPLETE 226
#define PASV 227
#define AUTH_SUCCESS 230
#define NEED_PASS 331

#define DEF_USER "anonymous"
#define DEF_PASS "password"

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

void send_command(int socket_fd, const char *command) {
    send(socket_fd, command, strlen(command), 0);
    printf("Sent: %s", command);
}

void receive_response(int socket_fd) {
    char buffer[MAX_BUFFER_SIZE];
    recv(socket_fd, buffer, sizeof(buffer), 0);
    printf("Received: %s", buffer);
}

void parse_url(const char *url, char *protocol, char *username, char *password, char *host, char *path) {
    const char *ptr = url;

    // Find protocol
    while (*ptr && *ptr != ':') {
        *protocol++ = *ptr++;
    }
    *protocol = '\0';

    // Skip "://"
    ptr += 3;

    // Check for username and password
    if (strstr(ptr, "@")) {
        // Username and password are present
        while (*ptr && *ptr != ':') {
            *username++ = *ptr++;
        }
        *username = '\0';

        // Skip ":"
        ptr++;

        while (*ptr && *ptr != '@') {
            *password++ = *ptr++;
        }
        *password = '\0';

        // Skip "@"
        ptr++;
    } else {
        // No username and password, use defaults
        strcpy(username, DEF_USER);
        strcpy(password, DEF_PASS);
    }

    // Find host
    while (*ptr && *ptr != '/' && *ptr != ':') {
        *host++ = *ptr++;
    }
    *host = '\0';

    // Find path
    if (*ptr == '/') {
        strcpy(path, ptr);
    } else {
        *path = '/';
        *(path + 1) = '\0';
    }
}