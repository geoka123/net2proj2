// client_windows.c
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define DURATION 30
#define INTERVAL 2

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    char *buffer = (char *)malloc(BUFFER_SIZE);
    memset(buffer, 'A', BUFFER_SIZE);  // Fill with dummy data

    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Connect failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    printf("Connected to server %s:%d\n", server_ip, port);

    time_t start_time = time(NULL);
    time_t next_report = start_time + INTERVAL;
    time_t end_time = start_time + DURATION;

    size_t total_bytes_sent = 0;
    size_t interval_bytes_sent = 0;

    while (time(NULL) < end_time) {
        int sent = send(sockfd, buffer, BUFFER_SIZE, 0);
        if (sent <= 0) {
            fprintf(stderr, "Send failed: %d\n", WSAGetLastError());
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

    closesocket(sockfd);
    WSACleanup();
    free(buffer);
    return 0;
}
