#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LENGTH          100
#define MAX_BUFFER_SIZE     1024
#define FTP_DATA_PORT       20
#define FTP_CONTROL_PORT    21

/* Server Responses */
#define PASSWORD_REQUIRED   331
#define PASV                227 // Passive mode
#define LOGOUT              221
#define LOGIN_SUCCESS       230
#define AUTH_READY          220
#define TRANSFER_COMPLETE   226
#define TRANSFER_READY      150
#define FILE_NOT_FOUND      550

void parseUrl(const char *url, char **username, char **password, char **hostname, char **path) {

    // Check for the presence of "://"
    const char *colonDoubleSlash = strstr(url, "://");
    if (colonDoubleSlash != NULL) {
        // Skip the "://"
        colonDoubleSlash += 3;

        // Check for the presence of "<user>:<password>@" and extract if present
        const char *at = strchr(colonDoubleSlash, '@');
        if (at != NULL) {
            // Calculate the length of the user info
            size_t userInfoLength = at - colonDoubleSlash;
            
            // Reallocate memory for username
            char *usernameInfo = malloc((userInfoLength + 1) * sizeof(char));

            strncpy(usernameInfo, colonDoubleSlash, userInfoLength);
            usernameInfo[userInfoLength] = '\0';  // Null-terminate the string

            printf("Username Info: %s\n", usernameInfo);

            // Move the pointer after '@'
            colonDoubleSlash = at + 1;

            // Find the ':' to separate username and password
            const char *colon = strchr(usernameInfo, ':');

            if (colon != NULL) {
                // Calculate the length of the username
                size_t usernameLength = colon - usernameInfo;

                if (username == NULL) {
                    printf("username is NULL\n");
                } else {
                    *username = realloc(*username, usernameLength + 1);
                    if (*username == NULL) {
                        printf("Failed to allocate memory\n");
                    } else {
                        strncpy(*username, usernameInfo, usernameLength);
                        (*username)[usernameLength] = '\0';  // Null-terminate the string
                        printf("Username: %s\n", *username);
                    }
                }

                // Calculate the length of the password
                size_t passwordLength = userInfoLength - usernameLength - 1;

                if (password == NULL) {
                    printf("password is NULL\n");
                } else {
                    // Reallocate memory for the password
                    *password = realloc(*password, passwordLength + 1);
                    if (*password == NULL) {
                        printf("Failed to allocate memory\n");
                    } else {
                        strncpy(*password, colon + 1, passwordLength);
                        (*password)[passwordLength] = '\0';  // Null-terminate the string
                        printf("Password: %s\n", *password);
                    }
                }
            }
        }

        // Find the first '/' after "://"
        const char *slash = strchr(colonDoubleSlash, '/');
        if (slash != NULL) {

            // Calculate the length of the hostname
            size_t hostnameLength = slash - colonDoubleSlash;

            if (hostname == NULL) {
                printf("hostname is NULL\n");
            } else {
                // Reallocate memory for hostname
                *hostname = realloc(*hostname, hostnameLength + 1);
                if (*hostname == NULL) {
                    printf("Failed to allocate memory\n");
                } else {
                    // Copy the hostname to the 'hostname' buffer
                    strncpy(*hostname, colonDoubleSlash, hostnameLength);
                    (*hostname)[hostnameLength] = '\0';  // Null-terminate the string
                    printf("Hostname: %s\n", *hostname);
                }
            }

            // Reallocate memory for path
            *path = realloc(*path, strlen(slash) + 1);

            // Copy the path to the 'path' buffer
            strcpy(*path, slash);
        } else {
            // No path is present, exit
            perror("Error, path was not provided");
            exit(EXIT_FAILURE);
        }
    } else {
        // Invalid URL format, exit
        perror("Error, invalid url format");
        exit(EXIT_FAILURE);
    }
}

const char* extractFileNameFromPath(const char* path) {
    // Find the last occurrence of the path separator ('/') in the given path
    const char* lastSlash = strrchr(path, '/');
    // If the path separator is found, return the string after the last '/'
    if (lastSlash != NULL) {
        return lastSlash + 1;
    } else {
        // If no path separator is found, return the entire path
        return path;
    }
}

char *convertHostnameToIp(const char *hostname) {
    struct hostent *h;

    if ((h = gethostbyname(hostname)) == NULL) {
        herror("gethostbyname()");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the IP address as a string
    char *ip_address = malloc(INET_ADDRSTRLEN);
    if (ip_address == NULL) {
        perror("malloc()");
        exit(EXIT_FAILURE);
    }

    // Convert the first address in h_addr_list to a string
    inet_ntop(AF_INET, h->h_addr_list[0], ip_address, INET_ADDRSTRLEN);

    return ip_address;
}

int establishControlConnectionToServer(const char *server_ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /* server address handling */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);

    /* open a TCP socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* connect to the server */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void loginToFTPHost(int control_socket, char **username, char **password) {
    char buffer[MAX_BUFFER_SIZE];
    size_t bytes;

    // Receive the welcome message from the server
    bytes = recv(control_socket, buffer, sizeof(buffer), 0);
    buffer[bytes] = '\0';
    printf("\nServer Response: %s", buffer);

    // Send the USER command if username is not NULL
    if (*username == NULL) {
        // Prompt the user for the username
        printf("\nUSER ");
        scanf("%s", buffer);
        *username = (char*)buffer; // Make a copy to avoid modifying the original string
    }

    snprintf(buffer, sizeof(buffer), "USER %s\r\n", *username);
    bytes = send(control_socket, buffer, strlen(buffer), 0);

    // Receive the response for the USER command
    bytes = recv(control_socket, buffer, sizeof(buffer), 0);
    buffer[bytes] = '\0';
    printf("Server Response: %s", buffer);

    // Check if the server accepted the username
    if (strstr(buffer, "331") == NULL) {
        perror("Invalid username");
        exit(EXIT_FAILURE);
    }

    // Send the PASS command if password is not NULL
    if (*password == NULL) {
        // Prompt the user for the password
        printf("\nPASS ");
        scanf("%s", buffer);
        *password = (char*)buffer;
    }

    snprintf(buffer, sizeof(buffer), "PASS %s\r\n", *password);
    bytes = send(control_socket, buffer, strlen(buffer), 0);

    // Receive the response for the PASS command
    bytes = recv(control_socket, buffer, sizeof(buffer), 0);
    buffer[bytes] = '\0';
    printf("Server Response: %s", buffer);

    // Check if the server accepted the password
    if (strstr(buffer, "230") == NULL) {
        perror("Invalid password");
        exit(EXIT_FAILURE);
    }
}

int enterPassiveModeAndGetPort(int control_socket) {
    char buffer[MAX_BUFFER_SIZE];
    size_t bytes;

    printf("\nPASV\n");

    // Send the PASV command
    snprintf(buffer, sizeof(buffer), "PASV\r\n");
    bytes = send(control_socket, buffer, strlen(buffer), 0);

    // Receive the response for the PASV command
    bytes = recv(control_socket, buffer, sizeof(buffer), 0);
    buffer[bytes] = '\0';
    printf("Server Response: %s", buffer);

    // Parse the response to extract the data port information
    unsigned int ip_part1, ip_part2, ip_part3, ip_part4, port_high, port_low;
    sscanf(buffer, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)", &ip_part1, &ip_part2, &ip_part3, &ip_part4, &port_high, &port_low);

    // Calculate the data port
    int data_port = port_high * 256 + port_low;
    return data_port;
}

void handleError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void sendCommand(int socket, const char *command, const char *arg) {
    char buffer[MAX_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s %s\r\n", command, arg);
    if (send(socket, buffer, strlen(buffer), 0) == -1) {
        handleError("Error sending command");
    }
}

void receiveResponse(int socket, char *buffer, size_t size) {
    size_t bytes = recv(socket, buffer, size - 1, 0);
    if (bytes == -1) {
        handleError("Error reading socket");
    }
    buffer[bytes] = '\0';
    printf("Server Response: %s", buffer);
}

void downloadFile(int controlSocket, int dataSocket, const char *filepath) {
    printf("\nRETR %s\n", filepath);

    // Send the retrieve command to the server
    sendCommand(controlSocket, "RETR", filepath);

    // Check the response in the control socket
    char response[MAX_BUFFER_SIZE];
    receiveResponse(controlSocket, response, sizeof(response));

    // Extract the status code from the response
    int firstStatusCode;
    sscanf(response, "%d", &firstStatusCode);

    if (firstStatusCode == TRANSFER_READY) {
        printf("File status okay, about to open data connection.\n");
    } else {
        handleError("Error getting the file status. Connection can't be established\n");
    }

    const char *filename = extractFileNameFromPath(filepath);
    printf("Filename: %s\n", filename);
    // Receive the file from the server and store it locally
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        handleError("Error opening file for writing");
    }

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = recv(dataSocket, buffer, sizeof(buffer), 0)) > 0) {
        if (fwrite(buffer, 1, bytesRead, file) != bytesRead) {
            handleError("Error writing to file");
        }
    }

    if (bytesRead == -1) {
        handleError("Error reading from data socket");
    }

    fclose(file);
    close(dataSocket);

    // Check the control socket for success or failure
    receiveResponse(controlSocket, response, sizeof(response));

    printf("\nServer response: %s", response);

    // Extract the status code from the response
    int secondStatusCode;
    sscanf(response, "%d", &secondStatusCode);

    if (secondStatusCode == TRANSFER_COMPLETE) {
        printf("File retrieved successfully\n");
    } else if (secondStatusCode == FILE_NOT_FOUND) {
        printf("Error: File not found or permission denied\n");
    } else {
        printf("Error retrieving file. Status code: %d\n", secondStatusCode);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *hostname = NULL;
    char *path = NULL;
    char *username = NULL;
    char *password = NULL;

    printf("initiated variables\n");

    // Parse URL and extract username and password (if present)
    parseUrl(argv[1], &username, &password, &hostname, &path);

    printf("Username: %s\n", username);
    printf("Password: %s\n", password);

    printf("\nHostname: %s\n", hostname);
    printf("Path: %s\n", path);

    char *ftpServerIp = convertHostnameToIp(hostname);

    if (ftpServerIp != NULL) {
        printf("\nIP Address for %s: %s\n", argv[1], ftpServerIp);
    } else {
        perror("Error: Could not get the ip address\n");
        exit(EXIT_FAILURE);
    }

    int controlSocket = establishControlConnectionToServer(ftpServerIp, FTP_CONTROL_PORT);

    // With the connected control socket, proceed with FTP commands
    loginToFTPHost(controlSocket, &username, &password);

    // Enter passive mode and get the data port
    int dataPort = enterPassiveModeAndGetPort(controlSocket);
    printf("\nData Port: %d\n", dataPort);

    int dataSocket = establishControlConnectionToServer(ftpServerIp, dataPort);

    // Retrieve the file from the server
    downloadFile(controlSocket, dataSocket, path);

    // Close the sockets when done
    close(dataSocket);
    close(controlSocket);
    free(ftpServerIp);

    return 0;
}