/**
 * ============================================================================
 * Assignment 3: Minimal Local HTTP Web Server in C
 * ============================================================================
 * Description: Listens on port 8080, accepts TCP HTTP connections, parses the GET
 *              request header, and returns a standard HTTP/1.1 200 OK response with HTML body.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o http_server http_server.c
 *
 * HOW TO RUN:
 *   ./http_server
 *   (Keep running in background, or open http://127.0.0.1:8080/ in a browser / curl / http_client)
 *
 * HOW TO STOP:
 *   - Stop server: Press [Ctrl + C] in this terminal.
 *   - Force kill if stuck: killall http_server
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

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
    printf("Local HTTP Server active at http://127.0.0.1:%d/\n", PORT);
    printf("Press [Ctrl + C] to stop the server at any time.\n\n");

    const char *html_body = "<html><body><h1>Computer Networks Lab</h1><p>Student: Simiyon (3122247001062)</p><p>HTTP GET Successful!</p></body></html>\r\n";
    char response[2048];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n%s",
             strlen(html_body), html_body);

    while (1) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;

        char req[1024];
        int n = read(cfd, req, sizeof(req) - 1);
        if (n > 0) {
            req[n] = '\0';
            printf("[HTTP Request Received]\n%s\n", req);
        }

        write(cfd, response, strlen(response));
        close(cfd);
    }

    close(sfd);
    return 0;
}
