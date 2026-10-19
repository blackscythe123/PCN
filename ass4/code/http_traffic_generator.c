/**
 * ============================================================================
 * Assignment 4: HTTP Packet Traffic Generator for Wireshark Sniffing
 * ============================================================================
 * Description: Forks into a background server and client on loopback (lo). Generates
 *              HTTP GET with custom headers (User-Agent, If-Modified-Since) and
 *              receives HTTP/1.1 200 OK to allow clear packet capture in Wireshark.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o http_traffic http_traffic_generator.c
 *
 * HOW TO RUN:
 *   ./http_traffic
 *   (Before running, start Wireshark or tshark on the 'lo' loopback interface)
 *
 * HOW TO STOP:
 *   The program completes client-server exchange and exits automatically.
 *   To abort execution: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8088

void run_server() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr));
    listen(sfd, 2);

    printf("[Server] Waiting for HTTP request to capture in Wireshark...\n");
    int cfd = accept(sfd, NULL, NULL);

    char req[1024];
    int n = read(cfd, req, sizeof(req) - 1);
    if (n > 0) {
        req[n] = '\0';
        printf("[Server] Received Request:\n%s\n", req);
    }

    const char *resp = "HTTP/1.1 200 OK\r\n"
                       "Server: Apache/2.4 (Ubuntu)\r\n"
                       "Last-Modified: Mon, 19 Oct 2026 12:00:00 GMT\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: 45\r\n\r\n"
                       "<html><body>Wireshark Lab Captured</body></html>";
    write(cfd, resp, strlen(resp));
    close(cfd);
    close(sfd);
}

void run_client() {
    sleep(1); // Give server a moment to bind and listen
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);
    if (connect(cfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("Client connect failed");
        close(cfd);
        return;
    }

    const char *http_get = "GET /wireshark-labs/HTTP-wireshark-file1.html HTTP/1.1\r\n"
                           "Host: 127.0.0.1:8088\r\n"
                           "User-Agent: Mozilla/5.0 (Ubuntu Wireshark Lab)\r\n"
                           "Accept: text/html,application/xhtml+xml\r\n"
                           "Accept-Language: en-US,en;q=0.9\r\n"
                           "If-Modified-Since: Mon, 19 Oct 2026 10:00:00 GMT\r\n"
                           "Connection: close\r\n\r\n";
    write(cfd, http_get, strlen(http_get));

    char buf[1024];
    int n;
    while ((n = read(cfd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
    }
    close(cfd);
    printf("[Client] HTTP GET successfully sent and response received.\n");
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        run_client();
    } else {
        run_server();
        wait(NULL);
    }
    return 0;
}
