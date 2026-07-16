#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define SERVER_IP "127.0.0.1"

int main() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char roll[100];
    printf("Enter Roll Number: ");
    if (fgets(roll, sizeof(roll), stdin) != NULL) {
        roll[strcspn(roll, "\n")] = 0;

        socklen_t len = sizeof(servaddr);
        
        sendto(sockfd, roll, strlen(roll), 0, (const struct sockaddr *)&servaddr, len);

        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&servaddr, &len);
        if (n >= 0) {
            buffer[n] = '\0';
            printf("\n--- Student Details ---\n");
            printf("%s", buffer);
            printf("-----------------------\n");
        } else {
            perror("Receive failed");
        }
    }

    close(sockfd);
    return 0;
}
