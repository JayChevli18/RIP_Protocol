#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define NUM_NODES 4  // Number of nodes (routers) in the network
#define INF 16       // Infinity metric (unreachable)

typedef struct {
    int destination;
    int next_hop;
    int metric;
} RouteEntry;

typedef struct {
    int id;                   // Node ID
    RouteEntry table[NUM_NODES];  // Routing table for this node
} Node;

// Nodes in the network
Node nodes[NUM_NODES];

// Function to initialize nodes and routing tables
void initializeNodes() {
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i].id = i;
        for (int j = 0; j < NUM_NODES; j++) {
            nodes[i].table[j].destination = j;
            if (i == j) {
                nodes[i].table[j].metric = 0;
                nodes[i].table[j].next_hop = i;
            } else {
                nodes[i].table[j].metric = INF;
                nodes[i].table[j].next_hop = -1;
            }
        }
    }

    // Direct neighbors with known distances
    nodes[0].table[1].metric = 1;  // Node 0 to Node 1 with distance 1
    nodes[0].table[2].metric = 4;  // Node 0 to Node 2 with distance 4
    nodes[1].table[0].metric = 1;  // Node 1 to Node 0 with distance 1
    nodes[1].table[2].metric = 2;  // Node 1 to Node 2 with distance 2
    nodes[2].table[0].metric = 4;  // Node 2 to Node 0 with distance 4
    nodes[2].table[1].metric = 2;  // Node 2 to Node 1 with distance 2
    nodes[2].table[3].metric = 1;  // Node 2 to Node 3 with distance 1
    nodes[3].table[2].metric = 1;  // Node 3 to Node 2 with distance 1
}

// Function to simulate sending routing information from one node to another
void sendRoutingUpdate(int from, int to) {
    for (int i = 0; i < NUM_NODES; i++) {
        if (i != from) {  // Split horizon (do not advertise back to the sender)
            int new_metric = nodes[from].table[i].metric + nodes[from].table[to].metric;
            if (new_metric < nodes[to].table[i].metric) {
                nodes[to].table[i].metric = new_metric;
                nodes[to].table[i].next_hop = from;
            }
        }
    }
}

// Function to simulate the periodic routing table updates between nodes
void updateRoutingTables() {
    for (int i = 0; i < NUM_NODES; i++) {
        for (int j = 0; j < NUM_NODES; j++) {
            if (i != j && nodes[i].table[j].metric != INF) {
                sendRoutingUpdate(i, j);
            }
        }
    }
}

// Function to print the routing table of each node
void printRoutingTables() {
    for (int i = 0; i < NUM_NODES; i++) {
        printf("\nRouting Table for Node %d:\n", i);
        printf("Destination\tNext Hop\tMetric\n");
        for (int j = 0; j < NUM_NODES; j++) {
            printf("%d\t\t%d\t\t%d\n", nodes[i].table[j].destination,
                   nodes[i].table[j].next_hop, nodes[i].table[j].metric);
        }
    }
}

int main() {
    initializeNodes();
    printf("Initial Routing Tables:");
    printRoutingTables();

    // Simulate a few iterations of routing table updates
    for (int i = 0; i < 5; i++) {
        printf("\nIteration %d of routing update:\n", i + 1);
        updateRoutingTables();
        printRoutingTables();
    }

    return 0;
}