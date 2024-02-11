#include "SocketUtil.h"
#define MAX_BUFFER_SIZE 1024
void handleClientRequest(int clientSocketFD);

// Function to determine content type based on file extension
const char *getContentType(const char *filePath) {
    const char *extension = strrchr(filePath, '.');
    if (extension != NULL) {
        if (strcmp(extension, ".html") == 0 || strcmp(extension, ".htm") == 0) {
            return "text/html";
        } else if (strcmp(extension, ".txt") == 0) {
            return "text/plain";
        } else if (strcmp(extension, ".jpg") == 0 || strcmp(extension, ".jpeg") == 0) {
            return "image/jpeg";
        } else if (strcmp(extension, ".png") == 0) {
            return "image/png";
        } else if (strcmp(extension, ".gif") == 0) {
            return "image/gif";
        } else if (strcmp(extension, ".mp4") == 0) {
            return "video/mp4";
        } else if (strcmp(extension, ".mp3") == 0) {
            return "audio/mpeg";
        } // Add more file types as needed
    }
    return "application/octet-stream"; // Default content type
}

// Function to send file content to the client
int sendFile(int clientSocketFD, const char *filePath) {
    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Determine content type based on file extension
    const char *contentType = getContentType(filePath);
    const char *httpHeaderTemplate = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n";
    char httpHeader[1024];
    snprintf(httpHeader, sizeof(httpHeader), httpHeaderTemplate, contentType);
    write(clientSocketFD, httpHeader, strlen(httpHeader));

    // Read and send the file content
    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(clientSocketFD, buffer, bytesRead) == -1) {
            perror("Error sending file content to client");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

// Function to execute PHP script and send output to the client
int executePhpScript(int clientSocketFD, const char *filePath) {
    char phpCommand[255];
    snprintf(phpCommand, sizeof(phpCommand), "php -f ./%s", filePath);
    FILE *phpOutput = popen(phpCommand, "r");
    printf("Executing PHP command: %s\n", phpCommand);

    if (phpOutput == NULL) {
        perror("Error executing PHP script");
        return -1;
    }

    // Send HTTP response header with appropriate content type
    const char *contentType = getContentType(filePath);
    const char *httpHeaderTemplate = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n";
    char httpHeader[1024];
    snprintf(httpHeader, sizeof(httpHeader), httpHeaderTemplate, contentType);
    write(clientSocketFD, httpHeader, strlen(httpHeader));

    // Send the PHP script's output to the client
    char outputBuffer[1024];
    while (fgets(outputBuffer, sizeof(outputBuffer), phpOutput) != NULL) {
        printf("%s", outputBuffer);
        if (write(clientSocketFD, outputBuffer, strlen(outputBuffer)) == -1) {
            perror("Error sending PHP output to client");
            break;
        }
    }

    pclose(phpOutput);
    return 0;
}

int main() {
    int serverSocketFD = CreateTCPIpv4Socket();
    struct sockaddr_in *address = CreateIPv4Address("", 880); // Port 800 for HTTP

    int result = bind(serverSocketFD, (struct sockaddr *)address, sizeof(*address));

    if (result == 0) {
        printf("Socket was bound successfully\n");
    } else {
        perror("Error connecting to server");
        exit(1);
    }

    int listenResult = listen(serverSocketFD, 10); // Willing to accept incoming connections 10.
    // Ignore SIGPIPE signal
    signal(SIGPIPE, SIG_IGN);
    while (1) { // Run indefinitely
        struct sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientAddress, (&clientAddressSize));

        if (clientSocketFD == -1) {
            perror("Error accepting client connection");
            continue;
        }

        char buffer[1024];
        int bytesRead = recv(clientSocketFD, buffer, 1024, 0);

        if (bytesRead == 0) {
            // Client closed the connection
            printf("Client closed the connection.\n");
            close(clientSocketFD);
            continue;  // Continue to the next iteration of the loop
        } else if (bytesRead < 0) {
            perror("Error receiving data");
            close(clientSocketFD);
            continue;
        }

        char requestType[10];
        char filePath[255];
        sscanf(buffer, "%s %s", requestType, filePath);


        // Remove leading '/' from the file path if present
        if (filePath[0] == '/' && strlen(filePath) > 1) {
            memmove(filePath, filePath + 1, strlen(filePath));
        }

        printf("Request Type: %s\n", requestType);
        printf("File Path: %s\n", filePath);

        if (access(filePath, F_OK) == 0) {
            if (strstr(filePath, ".php") != NULL) {
                // Handle PHP files
                if (executePhpScript(clientSocketFD, filePath) == -1) {
                    perror("Error handling PHP script");
                }
            } else {
                // Send other types of files
                if (sendFile(clientSocketFD, filePath) == -1) {
                    perror("Error sending file");
                }
            }
        } else {
            // If the file does not exist, return a 404 Not Found response
            const char *notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
            write(clientSocketFD, notFoundResponse, strlen(notFoundResponse));

            perror("File not found");
        }

        close(clientSocketFD);
    }

    close(serverSocketFD);

    return 0;
}
