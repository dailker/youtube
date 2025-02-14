#ifndef P2P_NODE_H
#define P2P_NODE_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

typedef void (*logger_fn)(const char* message);

typedef struct {
    logger_fn trace;
    logger_fn debug;
    logger_fn info;
    logger_fn warn;
    logger_fn error;
    logger_fn fatal;
} Logger;

typedef struct {
    char address[INET_ADDRSTRLEN];
    int port;
} SeedNode;

typedef struct {
    char address[INET_ADDRSTRLEN];
    int port;
    int minPeerNo;
    int maxPeerNo;
    SeedNode* seeds;
    int seedCount;
    int pingTimeout;
    Logger logger;
} NodeConfig;

typedef struct Node {
    char address[INET_ADDRSTRLEN];
    int port;
    int minPeerNo;
    int maxPeerNo;
    int pingTimeout;
    Logger logger;
    int serverSocket;
    struct Peer* peers;
    SeedNode* seeds;
    int seedCount;
} Node;

void* create_node(NodeConfig* config);
void destroy_node(void* node);
bool connect_to_peer(void* node, const char* address, int port);
void disconnect_from_peer(void* node, const char* address, int port);

Logger create_default_logger();

#endif // P2P_NODE_H