#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <libgen.h>

void parse_url(const char *url, char *host, int *port, char *path) {
    strcpy(path, "/");
    *port = 80;

    const char *p = url;
    if (strncmp(p, "http://", 7) == 0) {
        p += 7;
    } else if (strncmp(p, "https://", 8) == 0) {
        p += 8;
        *port = 443;
    }

    const char *slash = strchr(p, '/');
    int host_len;
    if (slash) {
        host_len = slash - p;
        strcpy(path, slash);
    } else {
        host_len = strlen(p);
    }

    char host_port[256];
    strncpy(host_port, p, host_len);
    host_port[host_len] = '\0';

    char *colon = strchr(host_port, ':');
    if (colon) {
        *colon = '\0';
        *port = atoi(colon + 1);
    }
    strcpy(host, host_port);
}

int main() {
    char url[1024];
    printf("Enter URL: ");
    if (!fgets(url, sizeof(url), stdin)) {
        return -1;
    }
    url[strcspn(url, "\r\n")] = '\0';

    char host[256];
    int port;
    char path[1024];
    parse_url(url, host, &port, path);

    printf("Client extracts:\n");
    printf("Host : %s\n", host);
    printf("Port : %d\n", port);
    printf("Path : %s\n\n", path);

    struct hostent *server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Error: No such host '%s'\n", host);
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error opening socket");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting");
        close(sock);
        return -1;
    }

    char request[2048];
    sprintf(request,
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "\r\n",
            path, host);

    printf("Then sends:\n%s", request);

    char path_copy[1024];
    strcpy(path_copy, path);
    char *base = basename(path_copy);
    char out_filename[512];

    if (strcmp(base, "/") == 0 || strlen(base) == 0 || strcmp(base, ".") == 0) {
        strcpy(out_filename, "downloaded_index.html");
    } else {
        sprintf(out_filename, "downloaded_%s", base);
    }

    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Error sending request");
        close(sock);
        return -1;
    }

    FILE *fp = fopen(out_filename, "wb");
    if (!fp) {
        perror("Error opening output file");
        close(sock);
        return -1;
    }

    char buffer[4096];
    int bytes_read;
    int first_byte = 1;
    int header_done = 0;
    char header_buf[8192];
    size_t header_len = 0;
    char last_four[4] = {0};
    size_t total_body_bytes = 0;

    struct timeval t_start, t_first, t_end;
    gettimeofday(&t_start, NULL);

    while ((bytes_read = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        if (first_byte) {
            gettimeofday(&t_first, NULL);
            first_byte = 0;
        }

        if (!header_done) {
            for (int i = 0; i < bytes_read; i++) {
                last_four[0] = last_four[1];
                last_four[1] = last_four[2];
                last_four[2] = last_four[3];
                last_four[3] = buffer[i];

                if (header_len < sizeof(header_buf) - 1) {
                    header_buf[header_len++] = buffer[i];
                }

                if (memcmp(last_four, "\r\n\r\n", 4) == 0) {
                    header_done = 1;
                    header_buf[header_len] = '\0';
                    int remaining = bytes_read - (i + 1);
                    if (remaining > 0) {
                        fwrite(buffer + i + 1, 1, remaining, fp);
                        total_body_bytes += remaining;
                    }
                    break;
                }
            }
        } else {
            fwrite(buffer, 1, bytes_read, fp);
            total_body_bytes += bytes_read;
        }
    }
    gettimeofday(&t_end, NULL);
    fclose(fp);
    close(sock);

    if (first_byte) {
        printf("Error: No data received from server.\n");
        unlink(out_filename);
        return -1;
    }

    double response_time = (t_first.tv_sec - t_start.tv_sec) * 1000.0 + 
                           (t_first.tv_usec - t_start.tv_usec) / 1000.0;
    double download_time = (t_end.tv_sec - t_first.tv_sec) * 1000.0 + 
                           (t_end.tv_usec - t_first.tv_usec) / 1000.0;
    
    double download_time_sec = download_time / 1000.0;
    double throughput = 0;
    if (download_time_sec > 0) {
        throughput = (total_body_bytes / 1024.0) / download_time_sec;
    }

    printf("\n--- Performance Metrics ---\n");
    printf("Response Time : %.3f ms\n", response_time);
    printf("Download Time : %.3f ms (%.6f seconds)\n", download_time, download_time_sec);
    printf("Data Size     : %zu bytes (%.2f KB)\n", total_body_bytes, total_body_bytes / 1024.0);
    if (download_time_sec > 0) {
        printf("Throughput    : %.2f KB/s\n", throughput);
    } else {
        printf("Throughput    : Instantaneous\n");
    }
    printf("Saved resource to: %s\n", out_filename);
    printf("---------------------------\n");

    return 0;
}
