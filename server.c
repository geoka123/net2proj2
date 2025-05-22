// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 9000            // Default port number (can be changed)
#define BUFFER_SIZE 10000    // Size of each data chunk (10 KB)
#define DURATION 30          // Total test duration in seconds
#define INTERVAL 2           // Report throughput every 2 seconds

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow quick reuse of port
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Server address config
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept from any interface

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Allocate data buffer
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    memset(buffer, 0, BUFFER_SIZE);  // Fill with zeros

    while (1) {
        // Accept a single client
        printf("Waiting for client...\n");
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Start sending data for DURATION seconds
        time_t start_time = time(NULL);
        time_t next_report = start_time + INTERVAL;
        time_t end_time = start_time + DURATION;

        size_t total_bytes_sent = 0;
        size_t interval_bytes_sent = 0;

        while (time(NULL) < end_time) {
            ssize_t sent = send(client_fd, buffer, BUFFER_SIZE, 0);
            if (sent <= 0) {
                perror("send failed or client disconnected");
                break;
            }

            total_bytes_sent += sent;
            interval_bytes_sent += sent;

            if (time(NULL) >= next_report) {
                double mbps = (interval_bytes_sent * 8.0) / (INTERVAL * 1000000.0);
                printf("[INTERVAL] Sent %.2f Mbps\n", mbps);
                interval_bytes_sent = 0;
                next_report += INTERVAL;
            }
        }

        double total_mbps = (total_bytes_sent * 8.0) / (DURATION * 1000000.0);
        printf("[TOTAL] Sent %.2f Mbps in %d seconds\n", total_mbps, DURATION);

        close(client_fd);
        printf("Client disconnected.\n\n");
    }

    free(buffer);
    close(server_fd);
    return 0;
}
