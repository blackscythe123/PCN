CC = gcc
CFLAGS = -Wall -Wextra

all: http_client

http_client: http_client.c
	$(CC) $(CFLAGS) -o http_client http_client.c

assets:
	python3 CN_Web_Server/generate_assets.py

clean:
	rm -f http_client downloaded_*
