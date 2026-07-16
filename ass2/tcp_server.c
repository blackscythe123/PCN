#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080
#define CSV_FILE "students.csv"

typedef struct {
    int socket;
} client_t;

int search_student(const char *roll, char *response) {
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
            strcpy(temp_line, line);
            char *r = strtok(temp_line, ",");
            char *n = strtok(NULL, ",");
            char *d = strtok(NULL, ",");
            char *s = strtok(NULL, ",");
            char *c = strtok(NULL, ",");

            sprintf(response, "Roll Number : %s\nName        : %s\nDepartment  : %s\nSemester    : %s\nCGPA        : %s\n",
                    r ? r : "N/A", 
                    n ? n : "N/A", 
                    d ? d : "N/A", 
                    s ? s : "N/A", 
                    c ? c : "N/A");
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

void *client_handler(void *arg) {
    client_t *client = (client_t *)arg;
    int sock = client->socket;
    free(client);

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        // Strip trailing whitespaces or newlines
        buffer[strcspn(buffer, "\r\n ")] = 0;
        printf("[TCP Server] Received request for Roll Number: '%s'\n", buffer);

        char response[1024];
        search_student(buffer, response);

        send(sock, response, strlen(response), 0);
    }

    close(sock);
    return NULL;
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[TCP Server] Listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread_id;
        client_t *client = malloc(sizeof(client_t));
        client->socket = client_sock;

        if (pthread_create(&thread_id, NULL, client_handler, (void*)client) < 0) {
            perror("Thread creation failed");
            free(client);
            close(client_sock);
        }
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
