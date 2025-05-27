// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 9000
#define BUFFER_SIZE 10000
#define DURATION 30
#define INTERVAL 2

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for client...\n");
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        time_t start_time = time(NULL);
        time_t next_report = start_time + INTERVAL;
        time_t end_time = start_time + DURATION;

        size_t total_bytes_received = 0;
        size_t interval_bytes_received = 0;

        while (time(NULL) < end_time) {
            ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
            if (bytes <= 0) {
                perror("recv failed or client disconnected");
                break;
            }

            total_bytes_received += bytes;
            interval_bytes_received += bytes;

            if (time(NULL) >= next_report) {
                double mbps = (interval_bytes_received * 8.0) / (INTERVAL * 1000000.0);
                printf("[INTERVAL] Received %.2f Mbps\n", mbps);
                interval_bytes_received = 0;
                next_report += INTERVAL;
            }
        }

        double total_mbps = (total_bytes_received * 8.0) / (DURATION * 1000000.0);
        printf("[TOTAL] Received %.2f Mbps in %d seconds\n", total_mbps, DURATION);

        close(client_fd);
        printf("Client disconnected.\n\n");
    }

    free(buffer);
    close(server_fd);
    return 0;
}
