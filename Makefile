CC = clang
CFLAGS = -g

all: gap_buffer

clean:
	rm -f gap_buffer

gap_buffer: gap_buffer.c gap_buffer.h
	$(CC) $(CFLAGS) -o gap_buffer gap_buffer.c
