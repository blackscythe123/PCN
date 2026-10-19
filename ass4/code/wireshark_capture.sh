#!/bin/bash
# ==============================================================================
# Assignment 4: Wireshark CLI Sniffing & Display Filter Cheat Sheet
# ==============================================================================
# HOW TO RUN:
#   bash wireshark_capture.sh
#
# HOW TO STOP:
#   To stop tshark capture: Press [Ctrl + C]
# ==============================================================================

echo "=== Wireshark HTTP Inspection Command Cheat Sheet ==="
echo "1. Run CLI capture on loopback (lo) saving HTTP packets:"
echo "   sudo tshark -i lo -f 'tcp port 8088' -Y 'http' -T fields -e ip.src -e ip.dst -e http.request.method -e http.response.code"
echo ""
echo "2. Essential Wireshark Display Filters:"
echo "   - Filter HTTP only:                     http"
echo "   - Filter HTTP GET requests:             http.request.method == \"GET\""
echo "   - Filter HTTP Responses:                http.response.code == 200"
echo "   - Filter Conditional GET:               http.request.line contains \"If-Modified-Since\""
echo "   - Filter by Specific Port:              tcp.port == 8088"
echo "   - Follow TCP Stream:                    tcp.stream eq 0"
echo "   - Packet Retransmissions:               tcp.analysis.retransmission"
