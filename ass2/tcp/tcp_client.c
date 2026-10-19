/**
 * ============================================================================
 * Assignment 2 (TCP): TCP Client with RTT & Throughput Measurement
 * ============================================================================
 * Description: Connects to TCP Server, transmits a message, measures Round-Trip Time
 *              (RTT) via clock_gettime, and computes data throughput in Kbps.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o tcp_client tcp_client.c
 *
 * HOW TO RUN:
 *   ./tcp_client
 *
 * HOW TO STOP:
 *   - Normal execution: Sends message, receives ACK, prints metrics, and terminates automatically.
 *   - Abort execution: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("Socket creation failed"); return 1; }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &saddr.sin_addr);

    if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Connect failed. Is ./tcp_server running?");
        close(fd);
        return 1;
    }
    printf("Connected to TCP Server at %s:%d\n", SERVER_IP, PORT);

    char msg[BUFFER_SIZE] = "Hello Server from SSN CSE Lab 3122247001062\n";
    char reply[BUFFER_SIZE];

    // Measure RTT & Throughput
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    write(fd, msg, strlen(msg));
    int n = read(fd, reply, sizeof(reply) - 1);

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    if (n > 0) {
        reply[n] = '\0';
        printf("Server Reply: %s\n", reply);

        double rtt_ms = (t_end.tv_sec - t_start.tv_sec) * 1000.0 +
                        (t_end.tv_nsec - t_start.tv_nsec) / 1e6;
        double throughput_kbps = (n * 8.0) / (rtt_ms);

        printf("--- Performance Metrics ---\n");
        printf("Round-Trip Time (RTT) : %.3f ms\n", rtt_ms);
        printf("Throughput            : %.3f Kbps\n", throughput_kbps);
    }

    close(fd);
    return 0;
}
