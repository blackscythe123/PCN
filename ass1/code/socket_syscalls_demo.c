/**
 * ============================================================================
 * Assignment 1: POSIX Socket System Calls Lifecycle Demonstration
 * ============================================================================
 * Description: Demonstrates core socket system calls: socket(), setsockopt(),
 *              bind(), listen(), getsockname(), and close().
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o socket_demo socket_syscalls_demo.c
 *
 * HOW TO RUN:
 *   ./socket_demo
 *
 * HOW TO STOP:
 *   The program executes sequentially and terminates automatically (exit code 0).
 *   To abort execution at any time: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    printf("--- POSIX Socket System Calls Lifecycle Demo ---\n");

    // 1. socket() - Create TCP endpoint (IPv4, Byte Stream)
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) { perror("socket failed"); return 1; }
    printf("[1] socket() created with file descriptor: %d\n", sfd);

    // 2. setsockopt() - Allow immediate port reuse without TIME_WAIT errors
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. bind() - Associate socket with IP address and Port number
    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(9090),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    if (bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("bind failed");
        close(sfd);
        return 1;
    }
    printf("[2] bind() bound socket to 0.0.0.0:9090\n");

    // 4. listen() - Set socket into passive listening mode (backlog queue = 5)
    listen(sfd, 5);
    printf("[3] listen() waiting for incoming connection queue (max 5)...\n");

    // 5. getsockname() - Retrieve bound address details
    struct sockaddr_in bound_addr;
    socklen_t len = sizeof(bound_addr);
    getsockname(sfd, (struct sockaddr*)&bound_addr, &len);
    printf("[4] getsockname() verified port: %d\n", ntohs(bound_addr.sin_port));

    // 6. close() - Terminate socket and free resources
    close(sfd);
    printf("[5] close() closed socket descriptor.\n");
    return 0;
}
