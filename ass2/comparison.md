# Socket Programming Comparison: TCP vs UDP

This document provides a comparative analysis of the TCP-based and UDP-based student record retrieval implementations from Assignment 2.

## Comparison Matrix

| Parameter | TCP Implementation (1a) | UDP Implementation (1b) |
| :--- | :--- | :--- |
| **Connection Type** | **Connection-oriented**: Requires a three-way handshake (`connect()` and `accept()`) to establish a dedicated virtual channel before any data is sent. | **Connectionless**: No handshake or channel setup. Data is sent directly to the destination address via independent datagrams. |
| **Reliability** | **Highly Reliable**: Guarantees delivery using acknowledgments (ACKs), timeouts, and retransmissions. If data is lost, TCP automatically resends it. | **Unreliable**: No acknowledgments or automatic retransmission. If a packet is lost in transit, it is discarded, and no error is reported. |
| **Ordering** | **Guaranteed Order**: Incoming bytes are reassembled using sequence numbers. The receiver receives data in the exact order it was sent. | **No Ordering Guarantees**: Datagrams can arrive in any order or even be duplicated because each packet is routed independently. |
| **Error Recovery** | **Built-in Error Recovery**: Performs checksum verification, detects duplicate/missing segments, and manages flow/congestion control natively. | **No Error Recovery**: Only checks for corruption using simple checksums. If a packet is corrupted, it is silently dropped without recovery. |
| **Communication Overhead** | **High Overhead**: Larger header size (minimum 20 bytes), state tracking, handshaking, and teardown packets increase total overhead. | **Low Overhead**: Small header size (8 bytes), stateless operation, and minimal network congestion. |
| **Concurrency Model** | **Concurrent Server**: Spawns a dedicated thread (`pthread`) per client. Allows multiple clients to query student details concurrently. | **Iterative Server**: Handles one query at a time in a single loop. Fast, stateless packet processing renders multithreading unnecessary. |
