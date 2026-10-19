/**
 * ============================================================================
 * Assignment 6 (Bonus / Exam Scope): Address Resolution Protocol (ARP) Simulation
 * ============================================================================
 * Description: Simulates ARP resolution mechanism:
 *              1. Broadcasts ARP Request frame ("Who has IP X? Tell IP Y")
 *              2. Unicast ARP Reply from matching host ("IP X is at MAC Z")
 *              3. Dynamically updates local ARP cache table
 *
 * HOW TO COMPILE:
 *   gcc -Wall -Wextra -o arp_sim arp_sim.c
 *
 * HOW TO RUN:
 *   ./arp_sim
 *
 * HOW TO STOP:
 *   The program completes queries and exits automatically.
 *   To abort execution: Press [Ctrl + C]
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>

struct ARPEntry {
    char ip[16];
    char mac[18];
};

struct ARPEntry arp_table[] = {
    {"192.168.1.1",  "AA:BB:CC:DD:EE:01"},
    {"192.168.1.10", "AA:BB:CC:DD:EE:0A"},
    {"192.168.1.20", "AA:BB:CC:DD:EE:14"},
    {"192.168.1.50", "AA:BB:CC:DD:EE:32"}
};
int arp_count = 4;

void resolve_arp(const char *target_ip) {
    printf("\n[ARP Request Broadcast] Who has %s? Tell 192.168.1.1\n", target_ip);
    for (int i = 0; i < arp_count; i++) {
        if (strcmp(arp_table[i].ip, target_ip) == 0) {
            printf("[ARP Reply Unicast] %s is at %s\n", target_ip, arp_table[i].mac);
            printf(">> ARP Cache Updated: %s -> %s\n", target_ip, arp_table[i].mac);
            return;
        }
    }
    printf("[ARP Failed] Host %s unreachable (Request timed out).\n", target_ip);
}

int main() {
    printf("--- Address Resolution Protocol (ARP) Simulation ---\n");
    resolve_arp("192.168.1.10");
    resolve_arp("192.168.1.50");
    resolve_arp("192.168.1.99"); // Non-existent host
    return 0;
}
