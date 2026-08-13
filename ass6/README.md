# Assignment 6: Error Detection using Checksum and CRC

## Features
- **Checksum**: 8-bit words with 1's complement addition and end-around carry
- **CRC**: Cyclic Redundancy Check using generator polynomial G(x) = x⁴ + x + 1 (10011)
- **Socket Communication**: TCP-based data transmission between client and server

## Algorithm / Pseudocode

### Client Algorithm
```
1. Read error detection method from user (Checksum or CRC)
2. Read data input from user
3. Calculate checksum/CRC:
   - For Checksum: Sum all 8-bit words, add end-around carry, take 1's complement
   - For CRC: Perform polynomial division using G(x)=10011
4. Create TCP socket and connect to server
5. Send method indicator + data + checksum/CRC
6. Optional: Flip bit in data to demonstrate error detection
```

### Server Algorithm
```
1. Create TCP socket, bind to port 8080, listen for connections
2. Accept incoming client connection (threaded handling)
3. Receive method indicator and data with error detection code
4. Recalculate checksum/CRC using received data
5. Compare calculated vs received error detection code
6. Display result: "ERROR-FREE" if match, else "ERROR DETECTED"
```

## Compilation
```bash
make
```

## Running

### Terminal 1 - Start Server
```bash
./server
```

### Terminal 2 - Run Client
```bash
./client
```

## Usage Examples

### Checksum Demo
```
Error Detection Methods:
1. Checksum
2. CRC
Choose (1/2): 1

=== CHECKSUM MODE ===
Enter data: HELLO

[Intermediate Sum Calculation]
  Step 1: 0x48 + Sum = 0x0048
  Step 2: 0x45 + Sum = 0x008D
  ... (continues for each byte)
  1's Complement (Checksum): 0xXX

[Sent] Data with Checksum (0xXX)
```

### CRC Demo
```
Error Detection Methods:
1. Checksum
2. CRC
Choose (1/2): 2

=== CRC MODE ===
Enter data: TEST

[CRC Calculation with G(x)=10011]
  Final CRC: 0xXX

[Sent] Data with CRC (0xXX)

Demonstrate error detection? (y/n): y
Introduce bit error (flip bit at position 0-7): 0
[Modified] Bit flipped at position 0
[Sent] Modified Data (Error Injected)
```

## Server Output
```
[Server] Listening on port 8080...

[Server] Received: HELLO (6 bytes)
[CHECKSUM] Received: 0xXX, Calculated: 0xXX
[Result] ERROR-FREE

[Server] Received: TEST (5 bytes)
[CRC] Received: 0xXX, Calculated: 0xXX
[Result] ERROR DETECTED
```

## Cleaning Up
```bash
make clean
```
