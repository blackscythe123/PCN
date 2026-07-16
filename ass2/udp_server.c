#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define CSV_FILE "students.csv"

int search_student_udp(const char *roll, char *response) {
    FILE *file = fopen(CSV_FILE, "r");
    if (!file) {
        strcpy(response, "Error: Database file not found.\n");
        return 0;
    }

    char line[256];
    char temp_line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        strcpy(temp_line, line);

        char *token = strtok(temp_line, ",");
        if (token && strcmp(token, roll) == 0) {
            char *name = strtok(NULL, ",");
            sprintf(response, "Roll Number : %s\nStudent Name: %s\n", token, name ? name : "N/A");
            found = 1;
            break;
        }
    }

    fclose(file);
    if (!found) {
        strcpy(response, "Student Record Not Found\n");
    }
    return found;
}

int main() {
    int sockfd;
    char buffer[1024];
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("[UDP Server] Listening on port %d...\n", PORT);

    socklen_t len = sizeof(cliaddr);
    int n;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[strcspn(buffer, "\r\n ")] = 0;
        printf("[UDP Server] Received request for Roll Number: '%s'\n", buffer);

        char response[1024];
        search_student_udp(buffer, response);

        sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}
