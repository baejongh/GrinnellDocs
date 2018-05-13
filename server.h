#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "types.h"
#include "ui.h"

typedef struct thread_arg {
  int socket_fd;
  int client_number;
} thread_arg_t;

typedef struct node {
  FILE* output;
  int client_id;
  struct node* next;
} node_t;

void* client_thread_fn(void* p);
void client_doc_request_handler(client_pl_t* pl, FILE* reply_stream);
void send_client_payload(server_pl_t* pl, FILE* reply_stream);
int  send_client_doc_lines(server_pl_t* reply, FILE* reply_stream);
void send_client_doc_end_reply(server_pl_t* reply, FILE* reply_stream);
void send_client_doc_start_reply(server_pl_t* reply, FILE* reply_stream);
void send_empty_typed_reply(server_pl_t* reply, FILE* reply_stream, int reply_type);
void client_ping_handler(client_pl_t* pl, FILE* reply_stream);
void client_write_char_handler(client_pl_t* pl, FILE* reply_stream, int this_client_id);
void payload_handler(client_pl_t* pl, FILE* reply_stream, int client_id);
void compute_offset(FILE* file, int x, int y);
void server_file_update (FILE* file, client_pl_t* pl);
void relay_file_update(server_pl_t* reply, FILE* reply_stream);
void add_conn(int client_id, FILE* output);
void remove_conn(int client_id);
void broadcast_write_char(server_pl_t* reply, client_pl_t* pl, int this_client_id);