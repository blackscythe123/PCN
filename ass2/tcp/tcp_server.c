/**
 * ============================================================================
 * Assignment 2 (TCP): Minimal TCP Echo Server
 * ============================================================================
 * Description: Listens on port 8080, accepts TCP connection, reads client messages,
 *              echoes them back, and calculates bytes handled.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o tcp_server tcp_server.c
 *
 * HOW TO RUN:
 *   ./tcp_server
 *   (Keep this terminal running, then start ./tcp_client in a second terminal)
 *
 * HOW TO STOP:
 *   - Clean shutdown from client: Type "exit" in the client terminal.
 *   - Direct shutdown from server: Press [Ctrl + C] in this terminal.
 *   - Force kill if stuck: killall tcp_server  (or kill -9 $(pgrep tcp_server))
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) { perror("socket failed"); return 1; }

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    listen(sfd, 5);
    printf("TCP Server active and listening on port %d...\n", PORT);
    printf("Press [Ctrl + C] to stop the server at any time.\n\n");

    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);
    int cfd = accept(sfd, (struct sockaddr*)&caddr, &clen);
    if (cfd < 0) { perror("accept failed"); close(sfd); return 1; }

    printf("Accepted connection from %s:%d\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

    char buf[BUFFER_SIZE];
    while (1) {
        int n = read(cfd, buf, sizeof(buf) - 1);
        if (n <= 0) break;
        buf[n] = '\0';
        printf("Client: %s", buf);

        // Echo message back to client
        write(cfd, buf, n);

        if (strncmp(buf, "exit", 4) == 0) {
            printf("Received exit signal from client. Closing session.\n");
            break;
        }
    }

    close(cfd);
    close(sfd);
    printf("Server stopped cleanly.\n");
    return 0;
}
