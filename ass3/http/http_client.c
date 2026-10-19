/**
 * ============================================================================
 * Assignment 3: HTTP Web Client in C with Throughput & Latency
 * ============================================================================
 * Description: Connects via TCP to a local/remote HTTP server, sends a standard
 *              HTTP GET request, downloads the response, and measures RTT & throughput.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o http_client http_client.c
 *
 * HOW TO RUN:
 *   ./http_client
 *
 * HOW TO STOP:
 *   - Normal execution: Completes HTTP download, displays metrics, and exits automatically.
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
#define BUFFER_SIZE 4096

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) { perror("socket creation failed"); return 1; }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &saddr.sin_addr);

    if (connect(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Connection to HTTP server failed. Is ./http_server running?");
        close(sfd);
        return 1;
    }

    const char *get_req = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: C-HTTP-Client/1.0\r\nConnection: close\r\n\r\n";

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    write(sfd, get_req, strlen(get_req));

    char buf[BUFFER_SIZE];
    int total_bytes = 0;
    int n;

    printf("--- HTTP Response Received ---\n");
    while ((n = read(sfd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
        total_bytes += n;
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);
    close(sfd);

    double rtt_ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_nsec - t1.tv_nsec) / 1e6;
    double throughput_kbps = (total_bytes * 8.0) / (rtt_ms);

    printf("\n--- Performance Metrics ---\n");
    printf("Total Bytes Received  : %d bytes\n", total_bytes);
    printf("Round-Trip Time (RTT) : %.3f ms\n", rtt_ms);
    printf("Download Throughput   : %.3f Kbps\n", throughput_kbps);

    return 0;
}
