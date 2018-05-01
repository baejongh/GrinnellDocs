CC = clang
CFLAGS = -g -fsanitize=address

all: server client ui gap_buffer

clean:
	rm -f server
	rm -f client
	rm -f gap_buffer
	rm -f ui

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lpthread

client: client.c
	$(CC) $(CFLAGS) -o client client.c -lpthread

gap_buffer: gap_buffer.c gap_buffer.h
	$(CC) $(CFLAGS) -o gap_buffer gap_buffer.c

ui: ui.c ui.h
	$(CC) $(CFLAGS) -o ui ui.c -lncurses
