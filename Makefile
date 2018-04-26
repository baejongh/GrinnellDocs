CC = clang
CFLAGS = -g

all: server client

clean:
	rm -f server
	rm -f client

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lpthread

client: client.c
	$(CC) $(CFLAGS) -o client client.c -lpthread
