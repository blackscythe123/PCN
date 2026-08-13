// server.c

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX 100
#define RECORDS 5

// Structure for DNS records
struct DNSRecord {
  char domain[50];
  char ip[20];
};

// DNS Database
struct DNSRecord dnsTable[RECORDS] = {{"google.com", "142.250.183.14"},
                                      {"youtube.com", "142.250.66.206"},
                                      {"github.com", "140.82.114.4"},
                                      {"openai.com", "104.18.33.45"},
                                      {"ssn.edu.in", "103.27.235.45"}};

// Checks whether query is in valid domain format
int isValidDomain(char domain[]) { return strchr(domain, '.') != NULL; }

// Searches DNS table
char *resolveDomain(char domain[]) {

  for (int i = 0; i < RECORDS; i++) {

    if (strcmp(domain, dnsTable[i].domain) == 0)
      return dnsTable[i].ip;
  }

  return NULL;
}

// Sends reply to client
void sendReply(int sockfd, struct sockaddr_in client, socklen_t len,
               char message[]) {

  sendto(sockfd, message, strlen(message) + 1, 0, (struct sockaddr *)&client,
         len);
}

// Simulate timeout by intentionally not replying
void simulateTimeout() {

  printf("Simulating Timeout...\n");
  sleep(5);
}

int main() {

  int sockfd;
  char query[MAX];
  char reply[MAX];

  struct sockaddr_in server, client;
  socklen_t len = sizeof(client);

  // Create UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = INADDR_ANY;

  bind(sockfd, (struct sockaddr *)&server, sizeof(server));

  printf("====================================\n");
  printf("     UDP DNS Server Started\n");
  printf("     Listening on Port %d\n", PORT);
  printf("====================================\n");

  while (1) {

    memset(query, 0, MAX);

    recvfrom(sockfd, query, MAX, 0, (struct sockaddr *)&client, &len);

    printf("\nReceived Query : %s\n", query);

    // Timeout Simulation
    if (strcmp(query, "timeout.com") == 0) {

      simulateTimeout();
      continue;
    }

    // Invalid Format
    if (!isValidDomain(query)) {

      strcpy(reply, "ERROR: Invalid Format");

      sendReply(sockfd, client, len, reply);

      printf("Reply Sent : %s\n", reply);

      continue;
    }

    // DNS Lookup
    char *ip = resolveDomain(query);

    if (ip != NULL) {

      strcpy(reply, ip);

      printf("Domain Found\n");
    } else {

      strcpy(reply, "NXDOMAIN");

      printf("Domain Not Found\n");
    }

    sendReply(sockfd, client, len, reply);

    printf("Reply Sent : %s\n", reply);
  }

  close(sockfd);

  return 0;
}