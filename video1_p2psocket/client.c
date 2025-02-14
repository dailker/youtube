#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    const char* message = "Hello from client!";
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Message sent to server: %s\n", message);

    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';
    printf("Response from server: %s\n", buffer);

    close(sock);
    printf("Connection closed.\n");

    return 0;
}