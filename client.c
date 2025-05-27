// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 10000    // 10 KB
#define DURATION 30          // Total test duration
#define INTERVAL 2           // Report interval

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in server_addr;
    char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    memset(buffer, 'A', BUFFER_SIZE);  // Dummy data

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", server_ip, port);

    time_t start_time = time(NULL);
    time_t next_report = start_time + INTERVAL;
    time_t end_time = start_time + DURATION;

    size_t total_bytes_sent = 0;
    size_t interval_bytes_sent = 0;

    while (time(NULL) < end_time) {
        ssize_t sent = send(sockfd, buffer, BUFFER_SIZE, 0);
        if (sent <= 0) {
            perror("send failed");
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

    close(sockfd);
    free(buffer);
    return 0;
}
