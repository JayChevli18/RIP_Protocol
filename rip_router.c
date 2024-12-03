#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAX_ROUTERS 10
#define PORT_BASE 5000
#define UPDATE_INTERVAL 5

typedef struct {
    int destination;  // Destination router ID
    int next_hop;     // Next hop router ID
    int cost;         // Cost to destination
} RoutingEntry;

typedef struct {
    int router_id;         // Router ID
    RoutingEntry table[MAX_ROUTERS]; // Routing table entries
    int num_entries;       // Number of entries in the routing table
} Router;

typedef struct {
    int source_router;      // Source router ID
    RoutingEntry entries[MAX_ROUTERS]; // Routing table entries sent in RIP message
    int num_entries;        // Number of entries in the packet
} RIPPacket;

// Function to send a RIP packet
void send_rip_packet(int socket_fd, struct sockaddr_in *dest_addr, RIPPacket *packet) {
    sendto(socket_fd, packet, sizeof(RIPPacket), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr));
}

// Function to receive a RIP packet
void receive_rip_packet(int socket_fd, RIPPacket *packet) {
    recvfrom(socket_fd, packet, sizeof(RIPPacket), 0, NULL, NULL);
}

// Update the routing table based on received RIP packet
void update_routing_table(Router *router, RIPPacket *packet) {
    for (int i = 0; i < packet->num_entries; i++) {
        RoutingEntry *entry = &packet->entries[i];
        int found = 0;

        for (int j = 0; j < router->num_entries; j++) {
            if (router->table[j].destination == entry->destination) {
                found = 1;
                int new_cost = entry->cost + 1;
                if (new_cost < router->table[j].cost) {
                    router->table[j].cost = new_cost;
                    router->table[j].next_hop = packet->source_router;
                }
                break;
            }
        }

        if (!found) {
            router->table[router->num_entries].destination = entry->destination;
            router->table[router->num_entries].cost = entry->cost + 1;
            router->table[router->num_entries].next_hop = packet->source_router;
            router->num_entries++;
        }
    }
}

// Send periodic updates to neighbors
void send_periodic_updates(int socket_fd, struct sockaddr_in *neighbor_addrs, int num_neighbors, Router *router) {
    RIPPacket packet;
    packet.source_router = router->router_id;
    packet.num_entries = router->num_entries;
    memcpy(packet.entries, router->table, sizeof(router->table));

    for (int i = 0; i < num_neighbors; i++) {
        send_rip_packet(socket_fd, &neighbor_addrs[i], &packet);
    }
}

// Print the routing table
void print_routing_table(Router *router) {
    printf("\nRouter ID: %d\n", router->router_id);
    printf("Routing Table:\n");
    printf("Destination\tNext Hop\tCost\n");
    for (int i = 0; i < router->num_entries; i++) {
        printf("%d\t\t%d\t\t%d\n",
               router->table[i].destination,
               router->table[i].next_hop,
               router->table[i].cost);
    }
    printf("\n");
}

// Main function to simulate a single router
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <router_id> <neighbor_router_ids>...\n", argv[0]);
        return 1;
    }

    int router_id = atoi(argv[1]);
    int num_neighbors = argc - 2;
    int neighbor_ids[num_neighbors];

    for (int i = 0; i < num_neighbors; i++) {
        neighbor_ids[i] = atoi(argv[i + 2]);
    }

    // Initialize router
    Router router;
    router.router_id = router_id;
    router.num_entries = 1;
    router.table[0].destination = router_id;
    router.table[0].next_hop = router_id;
    router.table[0].cost = 0;

    // Create UDP socket
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Bind socket to local address
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(PORT_BASE + router_id);

    if (bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Setup neighbor addresses
    struct sockaddr_in neighbor_addrs[num_neighbors];
    for (int i = 0; i < num_neighbors; i++) {
        memset(&neighbor_addrs[i], 0, sizeof(neighbor_addrs[i]));
        neighbor_addrs[i].sin_family = AF_INET;
        neighbor_addrs[i].sin_addr.s_addr = inet_addr("127.0.0.1");
        neighbor_addrs[i].sin_port = htons(PORT_BASE + neighbor_ids[i]);
    }

    // Main loop
    time_t last_update = time(NULL);

    while (1) {
        // Check for periodic updates
        if (time(NULL) - last_update >= UPDATE_INTERVAL) {
            send_periodic_updates(socket_fd, neighbor_addrs, num_neighbors, &router);
            last_update = time(NULL);
        }

        // Check for incoming packets
        struct timeval timeout = {1, 0}; // 1 second timeout
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket_fd, &read_fds);

        if (select(socket_fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            RIPPacket packet;
            receive_rip_packet(socket_fd, &packet);
            update_routing_table(&router, &packet);
            print_routing_table(&router);
        }
    }

    close(socket_fd);
    return 0;
}