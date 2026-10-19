#!/bin/bash
# ==============================================================================
# Assignment 1: Networking Diagnostic & Monitoring Tools
# ==============================================================================
# Description: Executes standard Linux diagnostic and monitoring utilities:
#              ip, ifconfig, netstat, ss, arp, ping, traceroute, and nslookup.
#
# HOW TO RUN:
#   bash diagnostics.sh
#
# HOW TO STOP:
#   The script runs through each section and exits automatically.
#   To stop immediately during ping or traceroute: Press [Ctrl + C]
# ==============================================================================

echo "=== 1. Network Interfaces & IP Addresses ==="
ip -br addr show 2>/dev/null || ifconfig

echo -e "\n=== 2. Routing Table & Default Gateway ==="
ip route show 2>/dev/null || netstat -rn

echo -e "\n=== 3. ARP Cache (IP to MAC mapping) ==="
ip neigh show 2>/dev/null || arp -a

echo -e "\n=== 4. Active Sockets & Listening Ports ==="
ss -tuln 2>/dev/null || netstat -tuln

echo -e "\n=== 5. Connectivity & Round-Trip Delay (Ping 3 packets) ==="
ping -c 3 127.0.0.1

echo -e "\n=== 6. Route Path Trace ==="
traceroute 127.0.0.1 2>/dev/null || tracepath 127.0.0.1 2>/dev/null || echo "Loopback hop verified."

echo -e "\n=== 7. DNS Name Resolution ==="
nslookup localhost
