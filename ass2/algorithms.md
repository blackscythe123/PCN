# Assignment 2: Student Database Sockets

### 1. TCP Server (`tcp_server.c`)
1. **Socket Initialization**: Initialize a TCP socket descriptor using `socket(AF_INET, SOCK_STREAM, 0)`.
2. **Endpoint Binding**: Bind the socket to port `8080` and `INADDR_ANY` using `bind()` to listen on all interfaces.
3. **Passive Listen State**: Enter passive listening mode via `listen()` with a connection backlog queue.
4. **Connection Acceptance**: Accept incoming three-way handshake connection requests (`accept()`) to spawn client-specific connection descriptors.
5. **Thread-Level Demultiplexing**: Hand off each client descriptor to a detached thread (`pthread_create()`, `pthread_detach()`) for parallel socket processing.
6. **Payload Retrieval & Search**: Read the client query data (`recv()`), parse the local database file, and retrieve student records.
7. **Response & Connection Teardown**: Encapsulate details in a TCP segment, transmit to client (`send()`), and initiate connection teardown using `close()`.

### 2. TCP Client (`tcp_client.c`)
1. **Socket Creation**: Initialize a TCP socket descriptor (`SOCK_STREAM`).
2. **Three-Way Handshake Connection**: Connect to the remote server endpoint at `127.0.0.1:8080` (`connect()`), initiating the TCP three-way handshake.
3. **Application Query Input**: Prompt the user to input the student Roll Number payload.
4. **Segment Transmission**: Encapsulate the query and transmit it over the active socket connection stream (`send()`).
5. **Segment Reception**: Enter a read loop to receive incoming packet streams (`recv()`) and print the response payload.
6. **Socket Close**: Perform client-side connection teardown (`close()`) and release socket resources.

### 3. UDP Server (`udp_server.c`)
1. **Socket Initialization**: Initialize a connectionless UDP socket descriptor using `socket(AF_INET, SOCK_DGRAM, 0)`.
2. **Endpoint Binding**: Bind the socket descriptor to port `8081` (`bind()`) to receive incoming Datagrams.
3. **Datagram Demultiplexing Loop**: Listen and receive stateless datagram packets (`recvfrom()`), capturing the client IP and ephemeral source port.
4. **Payload Processing**: Extract the Roll Number from the datagram payload and search the local database.
5. **Datagram Transmission**: Encapsulate the result into a UDP payload and send it back to the client's destination IP/port (`sendto()`).

### 4. UDP Client (`udp_client.c`)
1. **Input Payload**: Prompt the user for the Roll Number payload.
2. **Datagram Encapsulation**: Create a UDP socket descriptor (`SOCK_DGRAM`) and transmit the Roll Number payload to server at `127.0.0.1:8081` (`sendto()`).
3. **Stateless Reception**: Receive the response datagram (`recvfrom()`) containing the server payload.
4. **Client Termination**: Display the response data, close the socket descriptor (`close()`), and release resources.
