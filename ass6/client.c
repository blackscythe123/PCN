#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define GENERATOR 0x13

unsigned char checksum_calc(char *data, int len) {
    unsigned int sum = 0;
    printf("[Intermediate Sum Calculation]\n");
    for (int i = 0; i < len; i++) {
        sum += (unsigned char)data[i];
        printf("  Step %d: 0x%02X + Sum = 0x%04X\n", i + 1, (unsigned char)data[i], sum);
    }
    printf("  Total Sum: 0x%04X\n", sum);

    unsigned int carry = sum >> 8;
    sum = sum & 0xFF;
    printf("  Carry (End-around): 0x%02X\n", carry);

    sum += carry;
    printf("  After adding carry: 0x%04X\n", sum);

    unsigned char checksum = (unsigned char)(~sum & 0xFF);
    printf("  1's Complement (Checksum): 0x%02X\n", checksum);
    return checksum;
}

unsigned char crc_calc(char *data, int len) {
    unsigned char crc = 0;
    printf("[CRC Calculation with G(x)=10011]\n");
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ GENERATOR;
            else crc <<= 1;
            crc &= 0xFF;
        }
    }
    printf("  Final CRC: 0x%02X\n", crc);
    return crc;
}

void demo_checksum() {
    char data[1024];
    printf("\n=== CHECKSUM MODE ===\n");
    printf("Enter data: ");
    fgets(data, sizeof(data), stdin);
    data[strcspn(data, "\n")] = 0;

    int len = strlen(data);
    unsigned char checksum = checksum_calc(data, len);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) >= 0) {
        send(sock, "C", 1, 0);
        char packet[1025];
        strcpy(packet, data);
        packet[len] = checksum;
        send(sock, packet, len + 1, 0);
        printf("[Sent] Data with Checksum (0x%02X)\n", checksum);
        sleep(1);
    }
    close(sock);
}

void demo_crc() {
    char data[1024];
    printf("\n=== CRC MODE ===\n");
    printf("Enter data: ");
    fgets(data, sizeof(data), stdin);
    data[strcspn(data, "\n")] = 0;

    int len = strlen(data);
    unsigned char crc = crc_calc(data, len);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) >= 0) {
        send(sock, "R", 1, 0);
        char packet[1025];
        strcpy(packet, data);
        packet[len] = crc;
        send(sock, packet, len + 1, 0);
        printf("[Sent] Data with CRC (0x%02X)\n", crc);

        printf("\nDemonstrate error detection? (y/n): ");
        char choice;
        scanf("%c", &choice);
        if (choice == 'y' || choice == 'Y') {
            printf("Introduce bit error (flip bit at position 0-7): ");
            int pos;
            scanf("%d", &pos);
            if (pos >= 0 && pos < len) {
                packet[pos] ^= 0x01;
                printf("[Modified] Bit flipped at position %d\n", pos);

                int sock2 = socket(AF_INET, SOCK_STREAM, 0);
                connect(sock2, (struct sockaddr *)&addr, sizeof(addr));
                send(sock2, "R", 1, 0);
                send(sock2, packet, len + 1, 0);
                printf("[Sent] Modified Data (Error Injected)\n");
                sleep(1);
                close(sock2);
            }
        }
        sleep(1);
    }
    close(sock);
}

int main() {
    printf("Error Detection Methods:\n1. Checksum\n2. CRC\nChoose (1/2): ");
    int choice;
    scanf("%d\n", &choice);

    if (choice == 1) demo_checksum();
    else if (choice == 2) demo_crc();
    else printf("Invalid choice\n");

    return 0;
}
