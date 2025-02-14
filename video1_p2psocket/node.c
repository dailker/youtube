#include "node.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define DEFAULT_MIN_PEERS 3
#define DEFAULT_MAX_PEERS 5
#define DEFAULT_PING_TIMEOUT 3000

typedef struct Peer {
    int socket;
    struct sockaddr_in addr;
    bool connected;
    struct Peer* next;
} Peer;

void default_logger(const char* message) {
}

Logger create_default_logger() {
    Logger logger = {
        .trace = default_logger,
        .debug = default_logger,
        .info  = default_logger,
        .warn  = default_logger,
        .error = default_logger,
        .fatal = default_logger
    };
    return logger;
}

void* create_node(NodeConfig* config) {
    if (!config || !config->address[0] || config->port <= 0) {
        return NULL;
    }

    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;

    strncpy(node->address, config->address, INET_ADDRSTRLEN);
    node->port = config->port;
    node->minPeerNo = config->minPeerNo > 0 ? config->minPeerNo : DEFAULT_MIN_PEERS;
    node->maxPeerNo = config->maxPeerNo > 0 ? config->maxPeerNo : DEFAULT_MAX_PEERS;
    node->pingTimeout = config->pingTimeout > 0 ? config->pingTimeout : DEFAULT_PING_TIMEOUT;
    node->peers = NULL;

    node->logger = config->logger.info ? config->logger : create_default_logger();

    if (config->seeds && config->seedCount > 0) {
        node->seeds = malloc(sizeof(SeedNode) * config->seedCount);
        if (node->seeds) {
            memcpy(node->seeds, config->seeds, sizeof(SeedNode) * config->seedCount);
            node->seedCount = config->seedCount;
        } else {
            node->seedCount = 0;
        }
    } else {
        node->seeds = NULL;
        node->seedCount = 0;
    }

    node->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (node->serverSocket < 0) {
        node->logger.error("Failed to create server socket");
        free(node->seeds);
        free(node);
        return NULL;
    }

    int opt = 1;
    if (setsockopt(node->serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        node->logger.error("Failed to set socket options");
        close(node->serverSocket);
        free(node->seeds);
        free(node);
        return NULL;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(node->port);
    if (inet_pton(AF_INET, node->address, &server_addr.sin_addr) <= 0) {
        node->logger.error("Invalid address");
        close(node->serverSocket);
        free(node->seeds);
        free(node);
        return NULL;
    }

    if (bind(node->serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        node->logger.error("Bind failed");
        close(node->serverSocket);
        free(node->seeds);
        free(node);
        return NULL;
    }

    if (listen(node->serverSocket, node->maxPeerNo) < 0) {
        node->logger.error("Listen failed");
        close(node->serverSocket);
        free(node->seeds);
        free(node);
        return NULL;
    }

    node->logger.info("Node created successfully");
    return node;
}

void destroy_node(void* node_ptr) {
    if (!node_ptr) return;

    Node* node = (Node*)node_ptr;

    if (node->serverSocket >= 0) {
        close(node->serverSocket);
    }

    Peer* current = node->peers;
    while (current) {
        Peer* next = current->next;
        if (current->socket >= 0) {
            close(current->socket);
        }
        free(current);
        current = next;
    }

    free(node->seeds);
    free(node);
}

bool connect_to_peer(void* node_ptr, const char* address, int port) {
    if (!node_ptr || !address || port <= 0) return false;

    Node* node = (Node*)node_ptr;

    int peer_count = 0;
    Peer* current = node->peers;
    while (current) {
        if (current->connected) peer_count++;
        current = current->next;
    }

    if (peer_count >= node->maxPeerNo) {
        node->logger.warn("Maximum peer limit reached");
        return false;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        node->logger.error("Socket creation failed");
        return false;
    }

    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &peer_addr.sin_addr) <= 0) {
        node->logger.error("Invalid address");
        close(sock);
        return false;
    }

    if (connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        node->logger.error("Connection failed");
        close(sock);
        return false;
    }

    Peer* new_peer = malloc(sizeof(Peer));
    if (!new_peer) {
        node->logger.error("Memory allocation failed");
        close(sock);
        return false;
    }

    new_peer->socket = sock;
    new_peer->addr = peer_addr;
    new_peer->connected = true;
    new_peer->next = node->peers;
    node->peers = new_peer;

    node->logger.info("Successfully connected to new peer");
    return true;
}

void disconnect_from_peer(void* node_ptr, const char* address, int port) {
    if (!node_ptr || !address) return;
    
    Node* node = (Node*)node_ptr;
    Peer** current = &node->peers;
    
    while (*current) {
        Peer* peer = *current;
        char peer_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer->addr.sin_addr, peer_addr, INET_ADDRSTRLEN);
        
        if (strcmp(peer_addr, address) == 0 && ntohs(peer->addr.sin_port) == port) {
            *current = peer->next;
            close(peer->socket);
            free(peer);
            node->logger.info("Peer disconnected");
            return;
        }
        current = &peer->next;
    }
}
