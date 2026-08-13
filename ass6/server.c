#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080
#define GENERATOR 0x13

typedef struct { int sock; char type; } client_t;

unsigned char checksum_verify(char *data, int len) {
    unsigned int sum = 0;
    for (int i = 0; i < len; i++) sum += (unsigned char)data[i];
    while (sum >> 8) sum = (sum & 0xFF) + (sum >> 8);
    return (unsigned char)(~sum & 0xFF);
}

unsigned char crc_calc(char *data, int len) {
    unsigned char crc = 0;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ GENERATOR;
            else crc <<= 1;
            crc &= 0xFF;
        }
    }
    return crc;
}

void *client_handler(void *arg) {
    client_t *client = (client_t *)arg;
    int sock = client->sock;
    char type = client->type;
    free(client);

    char buffer[1024];
    int bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("\n[Server] Received: %s (%d bytes)\n", buffer, bytes_read);

        if (type == 'C') {
            unsigned char recv_checksum = buffer[bytes_read - 1];
            unsigned char calc_checksum = checksum_verify(buffer, bytes_read - 1);
            printf("[CHECKSUM] Received: 0x%02X, Calculated: 0x%02X\n", recv_checksum, calc_checksum);
            printf("[Result] %s\n", recv_checksum == calc_checksum ? "ERROR-FREE" : "ERROR DETECTED");
        } else {
            unsigned char recv_crc = buffer[bytes_read - 1];
            unsigned char calc_crc = crc_calc(buffer, bytes_read - 1);
            printf("[CRC] Received: 0x%02X, Calculated: 0x%02X\n", recv_crc, calc_crc);
            printf("[Result] %s\n", recv_crc == calc_crc ? "ERROR-FREE" : "ERROR DETECTED");
        }
    }
    close(sock);
    return NULL;
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("[Server] Listening on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
        if (client_sock >= 0) {
            char choice;
            recv(client_sock, &choice, 1, 0);

            pthread_t tid;
            client_t *c = malloc(sizeof(client_t));
            c->sock = client_sock;
            c->type = choice;

            pthread_create(&tid, NULL, client_handler, c);
            pthread_detach(tid);
        }
    }
    close(server_fd);
    return 0;
}
