#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "node.h"

volatile sig_atomic_t running = 1;

void handle_signal(int sig) {
    running = 0;
}

int main() {
    signal(SIGINT, handle_signal);
    
    NodeConfig config = {
        .address = "127.0.0.1",
        .port = 8080,
        .minPeerNo = 2,
        .maxPeerNo = 5,
        .pingTimeout = 3000,
        .logger = create_default_logger(),
        .seeds = NULL,
        .seedCount = 0
    };

    Node* node = (Node*)create_node(&config);
    if (!node) {
        printf("Failed to create node.\n");
        return EXIT_FAILURE;
    }

    printf("Node running on %s:%d. Press Ctrl+C to stop.\n", node->address, node->port);
    
    while (running) {
        sleep(1);
    }

    printf("Shutting down node...\n");
    destroy_node(node);
    printf("Node stopped.\n");

    return EXIT_SUCCESS;
}