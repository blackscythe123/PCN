/**
 * ============================================================================
 * Assignment 5: Domain Name Resolution (DNS) Server over UDP
 * ============================================================================
 * Description: Implements a UDP-based DNS server featuring:
 *              1. Fast in-memory cache lookup
 *              2. Authoritative DNS table fallback
 *              3. Automatic caching of resolved entries
 *              4. Proper NXDOMAIN error signaling for unknown domains
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o dns_server dns_server.c
 *
 * HOW TO RUN:
 *   ./dns_server
 *   (Keep this terminal running, then run ./dns_client in a second terminal)
 *
 * HOW TO STOP:
 *   - Stop server: Press [Ctrl + C] in this terminal.
 *   - Force kill if stuck: killall dns_server
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5353
#define BUFFER_SIZE 512

struct DNSRecord {
    char domain[64];
    char ip[16];
};

// Authoritative Database
struct DNSRecord db[] = {
    {"google.com", "142.250.190.46"},
    {"ssn.edu.in", "103.110.246.5"},
    {"github.com", "140.82.121.4"},
    {"localhost",  "127.0.0.1"}
};
int db_count = 4;

// Dynamic In-Memory Cache
struct DNSRecord cache[10];
int cache_count = 0;

const char* lookup_dns(const char *domain, int *from_cache) {
    // 1. Check Cache first
    for (int i = 0; i < cache_count; i++) {
        if (strcasecmp(cache[i].domain, domain) == 0) {
            *from_cache = 1;
            return cache[i].ip;
        }
    }
    *from_cache = 0;

    // 2. Query Authoritative Table
    for (int i = 0; i < db_count; i++) {
        if (strcasecmp(db[i].domain, domain) == 0) {
            if (cache_count < 10) {
                strcpy(cache[cache_count].domain, domain);
                strcpy(cache[cache_count].ip, db[i].ip);
                cache_count++;
            }
            return db[i].ip;
        }
    }
    return NULL; // NXDOMAIN
}

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

    printf("UDP DNS Server active on port %d...\n", PORT);
    printf("Press [Ctrl + C] to stop the server at any time.\n\n");

    char query[BUFFER_SIZE];
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);

    while (1) {
        int n = recvfrom(sfd, query, sizeof(query) - 1, 0, (struct sockaddr*)&caddr, &clen);
        if (n <= 0) continue;
        query[n] = '\0';
        query[strcspn(query, "\r\n")] = 0;

        int from_cache = 0;
        const char *ip = lookup_dns(query, &from_cache);
        char response[BUFFER_SIZE];

        if (ip) {
            snprintf(response, sizeof(response), "%s (Found in %s)", ip, from_cache ? "CACHE" : "DATABASE");
            printf("[DNS Query] %s -> %s\n", query, response);
        } else {
            strcpy(response, "NXDOMAIN: Domain Not Found");
            printf("[DNS Query] %s -> NXDOMAIN\n", query);
        }

        sendto(sfd, response, strlen(response), 0, (struct sockaddr*)&caddr, clen);
    }

    close(sfd);
    return 0;
}
