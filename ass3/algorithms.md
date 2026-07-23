# Assignment 3: HTTP Web Client & Server

---

## Part A: HTTP Client (`http_client.c`)

A C program that acts as an HTTP/1.1 client, connecting to a web server via raw TCP sockets to fetch a resource and report performance metrics.

1. **URI Parsing** (`parse_url`): Scan the input URL string for the scheme prefix (`http://` → port 80, `https://` → port 443). Extract the `hostname`, optional `port` override (e.g., `host:8080`), and resource `path` (defaults to `/`).
2. **Address Resolution**: Call `gethostbyname()` to perform a DNS lookup and resolve the hostname into a network IP address.
3. **TCP Connection**: Create a `SOCK_STREAM` socket and call `connect()` to establish a TCP session with the server at the resolved IP and port.
4. **HTTP Request Construction & Send**: Format and transmit an HTTP/1.1 GET request using `send()`:
   ```http
   GET /path HTTP/1.1
   Host: hostname
   Connection: close
   ```
5. **Response Stream Parsing**:
   - Record `t_start` (connection time) with `gettimeofday()`.
   - Call `recv()` in a loop; on the **first byte received**, record `t_first` to measure Response Time (RTT).
   - Scan the incoming byte stream for the `\r\n\r\n` delimiter to locate the end of HTTP response headers.
   - Write all bytes **after** the delimiter (the body payload) into a local output file (e.g., `downloaded_index.html`).
6. **Performance Metrics**: After the connection closes, record `t_end` and compute:
   - **Response Time** = `t_first − t_start` (ms)
   - **Download Time** = `t_end − t_first` (ms)
   - **Throughput** = body size (KB) / Download Time (s), printed in KB/s

---

## Part B: Python HTTP Server (`CN_Web_Server/`)

A lightweight static file server using Python's built-in `http.server` module to serve test assets for the HTTP client.

### Asset Generation (`generate_assets.py`)

Run once before starting the server to populate the serving directory:

1. **PDF Asset**: Search parent directories for an existing PDF file (`instructions.pdf`, `cheat_sheet.pdf`, `syllabus.pdf`). Copy it as `sample.pdf`. If none found, write a minimal valid PDF binary as a fallback.
2. **JPEG Asset**: Write a hardcoded 1×1 black-pixel JPEG byte sequence to `image.jpg`.

### Server Operation

The server is started from the `CN_Web_Server/` directory using the standard Python one-liner:
```bash
python3 -m http.server 8080
```
It serves the following static files to any connecting HTTP client:

| File        | Type       | Purpose                   |
|-------------|------------|---------------------------|
| `index.html`  | HTML       | Default webpage           |
| `notes.txt`   | Plain text | Plain text resource       |
| `image.jpg`   | JPEG       | Binary image resource     |
| `sample.pdf`  | PDF        | Binary document resource  |

The `http_client` (Part A) connects to `http://localhost:8080/<file>` to download and verify each resource type.

---

## Additional Problem: HTTPS Support

The `http_client.c` already parses `https://` URLs and sets the port to 443, but **does not implement TLS**. Extending it for HTTPS requires wrapping the raw TCP socket with an SSL/TLS layer using **OpenSSL**.

### Modified Algorithm for HTTPS

1. **URI Parsing** *(same as Part A)*: Detect `https://` scheme; set `port = 443` and set an `is_https` flag.
2. **Address Resolution & TCP Connection** *(same as Part A)*: `gethostbyname()` + `connect()` on port 443.
3. **TLS Handshake** *(HTTPS only)*:
   - Initialize the OpenSSL library: `SSL_library_init()`, `SSL_load_error_strings()`.
   - Create an SSL context: `SSL_CTX_new(TLS_client_method())`.
   - Wrap the connected TCP socket in an SSL object: `SSL_new(ctx)` → `SSL_set_fd(ssl, sock)`.
   - Perform the TLS handshake: `SSL_connect(ssl)`. This negotiates cipher suites, verifies the server certificate, and establishes an encrypted session.
   - *(Optional)* Verify the server certificate against the system CA store using `SSL_CTX_set_verify()` and `SSL_get_verify_result()`.
4. **HTTP Request Send** *(HTTPS only)*: Replace `send()` with `SSL_write(ssl, request, len)` to transmit the GET request over the encrypted channel.
5. **Response Stream Parsing** *(HTTPS only)*: Replace `recv()` with `SSL_read(ssl, buffer, sizeof(buffer))` to read the encrypted response. Header/body splitting and file writing logic is identical to Part A.
6. **Cleanup**: Call `SSL_shutdown(ssl)`, `SSL_free(ssl)`, `SSL_CTX_free(ctx)` before closing the socket.
7. **Performance Metrics** *(same as Part A)*: Response Time, Download Time, and Throughput are computed identically regardless of HTTP or HTTPS.

> **Note**: Compile with `-lssl -lcrypto` (OpenSSL) for HTTPS support. The current `Makefile` only compiles for plain HTTP.
