/**
 * ============================================================================
 * Assignment 6 (Part 1): Error Detection using 8-Bit One's Complement Checksum
 * ============================================================================
 * Description: Computes Internet checksum (one's complement of 8-bit sum with carry folding)
 *              for transmission, and verifies uncorrupted and corrupted packets.
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o checksum checksum.c
 *
 * HOW TO RUN:
 *   ./checksum
 *
 * HOW TO STOP:
 *   The program runs verification tests and exits automatically.
 *   To abort execution: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>

// Computes 8-bit One's Complement Checksum with carry folding
unsigned char compute_checksum(const char *data, int len) {
    unsigned int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += (unsigned char)data[i];
    }
    while (sum >> 8) {
        sum = (sum & 0xFF) + (sum >> 8);
    }
    return (unsigned char)(~sum);
}

// Verifies data against received checksum (sum + checksum == 0xFF)
int verify_checksum(const char *data, int len, unsigned char received_checksum) {
    unsigned int sum = received_checksum;
    for (int i = 0; i < len; i++) {
        sum += (unsigned char)data[i];
    }
    while (sum >> 8) {
        sum = (sum & 0xFF) + (sum >> 8);
    }
    return (sum == 0xFF);
}

int main() {
    char message[100] = "SSN-CSE-3122247001062";
    int len = strlen(message);

    printf("--- Checksum Generation (Sender) ---\n");
    printf("Original Data: %s (Length: %d bytes)\n", message, len);

    unsigned char checksum = compute_checksum(message, len);
    printf("Calculated Checksum: 0x%02X\n", checksum);

    printf("\n--- Checksum Verification (Receiver) ---\n");
    // Test 1: Uncorrupted packet
    if (verify_checksum(message, len, checksum)) {
        printf("[Test 1 - Valid Data] Checksum matches! Data integrity verified.\n");
    } else {
        printf("[Test 1 - Valid Data] Error detected!\n");
    }

    // Test 2: Injected 1-bit corruption
    message[0] ^= 1;
    printf("\nInjecting 1-bit error: modified 1st character to '%c'...\n", message[0]);
    if (verify_checksum(message, len, checksum)) {
        printf("[Test 2 - Corrupted] Undetected error!\n");
    } else {
        printf("[Test 2 - Corrupted] SUCCESS: Error detected via checksum mismatch!\n");
    }

    return 0;
}
