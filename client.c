// client.c

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define PORT 8080
#define MAX 100
#define CACHE_SIZE 10

// Structure for Cache
struct Cache {
  char domain[50];
  char ip[20];
};

struct Cache cache[CACHE_SIZE];
int cacheCount = 0;

// Search Cache
char *searchCache(char domain[]) {

  for (int i = 0; i < cacheCount; i++) {

    if (strcmp(domain, cache[i].domain) == 0)
      return cache[i].ip;
  }

  return NULL;
}

// Add entry to Cache
void addCache(char domain[], char ip[]) {

  if (cacheCount >= CACHE_SIZE)
    return;

  strcpy(cache[cacheCount].domain, domain);
  strcpy(cache[cacheCount].ip, ip);

  cacheCount++;
}

// Display Cache
void displayCache() {

  printf("\n----- Client Cache -----\n");

  if (cacheCount == 0) {
    printf("Cache Empty\n");
    return;
  }

  for (int i = 0; i < cacheCount; i++)
    printf("%s --> %s\n", cache[i].domain, cache[i].ip);

  printf("------------------------\n");
}

int main() {

  int sockfd;
  char domain[MAX], reply[MAX];

  struct sockaddr_in server;
  socklen_t len = sizeof(server);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");

  printf("=========== DNS Client ===========\n");

  while (1) {

    printf("\nEnter Domain (or exit): ");
    scanf("%s", domain);

    if (strcmp(domain, "exit") == 0)
      break;

    // Check Client Cache
    char *ip = searchCache(domain);

    if (ip != NULL) {

      printf("Cache Hit!\n");
      printf("IP Address : %s\n", ip);
      continue;
    }

    printf("Cache Miss! Querying Server...\n");

    sendto(sockfd, domain, strlen(domain) + 1, 0, (struct sockaddr *)&server,
           len);

    // Timeout using select()
    fd_set readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (select(sockfd + 1, &readfds, NULL, NULL, &tv) <= 0) {

      printf("Request Timed Out!\n");
      continue;
    }

    recvfrom(sockfd, reply, MAX, 0, (struct sockaddr *)&server, &len);

    if (strcmp(reply, "NXDOMAIN") == 0) {

      printf("Domain Not Found.\n");
    } else if (strcmp(reply, "ERROR: Invalid Format") == 0) {

      printf("Invalid Domain Format.\n");
    } else {

      printf("IP Address : %s\n", reply);

      addCache(domain, reply);
    }

    displayCache();
  }

  close(sockfd);

  return 0;
}