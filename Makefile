CC = clang
CFLAGS = -g

all: server client gap_buffer

clean:
	rm -f ./server/server
	rm -f ./client/client
	rm -f gap_buffer

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lpthread

client: client.c
	$(CC) $(CFLAGS) -o client client.c -lpthread

gap_buffer: gap_buffer.c gap_buffer.h
	$(CC) $(CFLAGS) -o gap_buffer gap_buffer.c
