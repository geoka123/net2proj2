// server_windows.c
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")  // Link against Winsock library

#define PORT 9000
#define BUFFER_SIZE 1024
#define DURATION 30
#define INTERVAL 2

int main() {
    WSADATA wsaData;
    SOCKET server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Allow reuse of port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        fprintf(stderr, "setsockopt failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(server_fd, 1) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    memset(buffer, 0, BUFFER_SIZE);

    while (1) {
        printf("Waiting for client...\n");
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        time_t start_time = time(NULL);
        time_t next_report = start_time + INTERVAL;
        time_t end_time = start_time + DURATION;

        size_t total_bytes_sent = 0;
        size_t interval_bytes_sent = 0;

        while (time(NULL) < end_time) {
            int sent = send(client_fd, buffer, BUFFER_SIZE, 0);
            if (sent <= 0) {
                fprintf(stderr, "Send failed or client disconnected: %d\n", WSAGetLastError());
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

        closesocket(client_fd);
        printf("Client disconnected.\n\n");
    }

    free(buffer);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
