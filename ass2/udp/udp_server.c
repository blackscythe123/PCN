/**
 * ============================================================================
 * Assignment 2 (UDP): UDP Echo Server
 * ============================================================================
 * Description: Listens on UDP port 9090, receives datagrams without establishing
 *              a connection, prints sender IP/port, and replies with an ACK packet.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o udp_server udp_server.c
 *
 * HOW TO RUN:
 *   ./udp_server
 *   (Keep this terminal running, then start ./udp_client in a second terminal)
 *
 * HOW TO STOP:
 *   - Stop server: Press [Ctrl + C] in this terminal.
 *   - Force kill if stuck: killall udp_server
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9090
#define BUFFER_SIZE 1024

int main() {
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0) { perror("socket failed"); return 1; }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("bind failed");
        close(sfd);
        return 1;
    }

    printf("UDP Server active on port %d...\n", PORT);
    printf("Press [Ctrl + C] to stop the server at any time.\n\n");

    char buf[BUFFER_SIZE];
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);

    while (1) {
        int n = recvfrom(sfd, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&caddr, &clen);
        if (n <= 0) break;
        buf[n] = '\0';
        printf("Received UDP datagram from %s:%d: %s\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port), buf);

        char ack[BUFFER_SIZE + 32];
        snprintf(ack, sizeof(ack), "ACK: %s", buf);
        sendto(sfd, ack, strlen(ack), 0, (struct sockaddr*)&caddr, clen);
    }

    close(sfd);
    printf("UDP Server stopped.\n");
    return 0;
}
