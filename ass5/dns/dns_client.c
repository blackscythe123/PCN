/**
 * ============================================================================
 * Assignment 5: Domain Name Resolution (DNS) Client over UDP
 * ============================================================================
 * Description: Sends UDP DNS query for a domain name to the local DNS server,
 *              receives the resolved IP (or NXDOMAIN), and measures resolution latency.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o dns_client dns_client.c
 *
 * HOW TO RUN:
 *   ./dns_client google.com
 *   ./dns_client ssn.edu.in
 *   ./dns_client invalid-domain.org
 *
 * HOW TO STOP:
 *   - Normal execution: Displays DNS resolution response, prints latency, and exits automatically.
 *   - Abort execution: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5353
#define BUFFER_SIZE 512

int main(int argc, char *argv[]) {
    const char *domain = (argc > 1) ? argv[1] : "google.com";

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("socket failed"); return 1; }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, SERVER_IP, &saddr.sin_addr);

    printf("Querying DNS Server (%s:%d) for: %s\n", SERVER_IP, PORT, domain);

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    sendto(fd, domain, strlen(domain), 0, (struct sockaddr*)&saddr, sizeof(saddr));

    char response[BUFFER_SIZE];
    socklen_t slen = sizeof(saddr);
    int n = recvfrom(fd, response, sizeof(response) - 1, 0, (struct sockaddr*)&saddr, &slen);

    clock_gettime(CLOCK_MONOTONIC, &t2);

    if (n > 0) {
        response[n] = '\0';
        double latency_ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_nsec - t1.tv_nsec) / 1e6;
        printf("\n--- DNS Resolution Response ---\n");
        printf("Domain  : %s\n", domain);
        printf("Result  : %s\n", response);
        printf("Latency : %.3f ms\n", latency_ms);
    } else {
        printf("Error: No response from DNS server.\n");
    }

    close(fd);
    return 0;
}
