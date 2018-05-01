CC = clang
CFLAGS = -g

all: server_new client_new ui gap_buffer

clean:
	rm -f server_new
	rm -f client_new
	rm -f gap_buffer
	rm -f ui

server: server_new.c
	$(CC) $(CFLAGS) -o server_new server_new.c -lpthread

client: client_new.c
	$(CC) $(CFLAGS) -o client_new client_new.c -lpthread

gap_buffer: gap_buffer.c gap_buffer.h
	$(CC) $(CFLAGS) -o gap_buffer gap_buffer.c

ui: ui.c ui.h
	$(CC) $(CFLAGS) -o ui ui.c
