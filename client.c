#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "types.h"
#include "ui.h"
#include "client.h"

server_info_t server_info;
bool active = true;
bool server_sending_file = false;
bool reply_received = false;

int main(int argc, char** argv) {
  if(argc != 4) {
    fprintf(stderr, "Usage: %s <server address> <port> <filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  server_info.addr = argv[1];
  server_info.port = atoi(argv[2]);
  server_info.output = NULL;
  server_info.input = NULL;

  // Create a thread for UI
  ui_fn_args_t* ui_args = (ui_fn_args_t*) malloc(sizeof(ui_fn_args_t));

  strcpy(ui_args->filename, argv[3]);

  pthread_t ui_thread;
  if (pthread_create(&ui_thread, NULL, ui_fn, ui_args)) {
    perror("pthread_create_failed");
    exit(EXIT_FAILURE);
  }

  server_loop(argv[3]);

  // Clean up the UI
  // TODO

  return 0;
}

void server_loop(char* filename) {
  // Connect to server
  init_server_streams();

  // Send CLIENT_DOC_REQUEST to open file on server
  client_pl_t* pl = (client_pl_t*) malloc(sizeof(client_pl_t));
  pl->msg_type = CLIENT_DOC_REQUEST;
  strcpy(pl->msg, filename);
  send_server_payload(pl);

  server_pl_t* reply = (server_pl_t*) malloc(sizeof(server_pl_t));

  while (fread(reply, sizeof(server_pl_t), 1, server_info.input) > 0) {
    
    if (reply->msg_type == SERVER_ECHO) {
      //printf("Client sent: %s\n", reply->msg);
    }
    payload_handler(reply);
  }

  free(pl);
}

// Connects to server
// Makes input and output streams available globally on server_info struct
void init_server_streams() {

  struct hostent* server = gethostbyname(server_info.addr);
  if (server == NULL) {
    fprintf(stderr, "Unable to find host");
    exit(EXIT_FAILURE);
  }

  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("Init socket failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(server_info.port)
  };

  bcopy((char*) server->h_addr_list[0], (char*) &addr.sin_addr.s_addr, server->h_length);

  if (connect(s, (struct sockaddr*) &addr, sizeof(struct sockaddr_in))) {
    perror("Socket connection failed");
    exit(2);
  }

  int s_copy = dup(s);
  if (s_copy == -1) {
    perror("Duplicating socket failed");
    exit(EXIT_FAILURE);
  }

  FILE* input  = fdopen(s, "r");
  FILE* output = fdopen(s_copy, "w");

  if (input == NULL || output == NULL) {
    perror("Opening socket file streams failed");
    exit(EXIT_FAILURE);
  }

  server_info.input  = input;
  server_info.output = output;

}

void* ui_fn(void* p) {
  ui_init_window();
  return NULL;
}

void send_server_payload(client_pl_t* pl) {
  if (server_info.output != NULL) {
    fwrite(pl, sizeof(client_pl_t), 1, server_info.output);
    fflush(server_info.output);
  }
}

void send_client_write_char_payload(int y_pos, int x_pos, char ch) {
  // Fill in payload fields
  client_pl_t* pl = (client_pl_t*) malloc(sizeof(client_pl_t));
  pl->msg_type = CLIENT_WRITE_CHAR;
  pl->x_pos = x_pos;
  pl->y_pos = y_pos;
  pl->ch = ch;

  // Send payload to server
  send_server_payload(pl);

  // Free memory for payload
  free(pl);
}

// Control flow for different message types sent from server
void payload_handler(server_pl_t* pl) {
  switch (pl->msg_type) {
    case SERVER_ECHO:
      // For debugging purposes.
      ui_write_line(pl->msg);
      break;
    case SERVER_DOC_START:
      server_sending_file = true;
      break;
    case SERVER_DOC_LINE:
      ui_write_line(pl->msg);
      break;
    case SERVER_DOC_END:
      // File contents have been sent; proceed as normal
      server_sending_file = false;
      break;
    case SERVER_DOC_NOT_FOUND:
      // We just write to an empty file 
      // (proceed without waiting for file contents)
      server_sending_file = false;
      break;
    case SERVER_WRITE_CHAR_RELAY:
      // For now, just add to the end of the file.
      // Later we can edit the location in the document that
      // needs to be changed
      ui_place_char(pl->ch, pl->x_pos, pl->y_pos);
      break;
    default:
      perror("Unexpected message type client.c payload_handler");
      break;
  }
}