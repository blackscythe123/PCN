/**
 * ============================================================================
 * Assignment 2 (UDP): UDP Client with Timeout & Retransmission
 * ============================================================================
 * Description: Implements reliable datagram transfer over UDP using socket
 *              receive timeouts (SO_RCVTIMEO) and automatic retransmissions (max 3 retries).
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o udp_client udp_client.c
 *
 * HOW TO RUN:
 *   ./udp_client
 *
 * HOW TO STOP:
 *   - Normal execution: Terminates automatically upon receiving an ACK or after 3 failed attempts.
 *   - Abort execution: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 9090
#define MAX_RETRIES 3
#define BUFFER_SIZE 1024

int main() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("socket creation failed"); return 1; }

    // Set 2-second receive timeout using SO_RCVTIMEO
    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &saddr.sin_addr);

    char msg[] = "UDP Packet Test - Reg 3122247001062";
    char reply[BUFFER_SIZE];
    int attempts = 0;
    int received = 0;

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    while (attempts < MAX_RETRIES && !received) {
        attempts++;
        printf("[Attempt %d/%d] Sending UDP packet...\n", attempts, MAX_RETRIES);
        sendto(fd, msg, strlen(msg), 0, (struct sockaddr*)&saddr, sizeof(saddr));

        socklen_t slen = sizeof(saddr);
        int n = recvfrom(fd, reply, sizeof(reply) - 1, 0, (struct sockaddr*)&saddr, &slen);

        if (n >= 0) {
            reply[n] = '\0';
            clock_gettime(CLOCK_MONOTONIC, &t2);
            printf("Received ACK from server: %s\n", reply);
            received = 1;

            double rtt_ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_nsec - t1.tv_nsec) / 1e6;
            printf("--- UDP Metrics ---\n");
            printf("Round-Trip Time (RTT) : %.2f ms\n", rtt_ms);
            printf("Retransmissions       : %d\n", attempts - 1);
        } else {
            printf("TIMEOUT! No ACK received within 2 seconds.\n");
        }
    }

    if (!received) {
        printf("Transmission FAILED: Server unresponsive after %d attempts.\n", MAX_RETRIES);
    }

    close(fd);
    return 0;
}
