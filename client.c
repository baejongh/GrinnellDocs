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

void server_loop();
void* ui_fn(void* p);
void init_server_streams();
server_pl_t* send_recieve_msg(client_pl_t* pl);
void send_server_payload(client_pl_t* pl);

server_info_t server_info;

int main(int argc, char** argv) {
  if(argc != 3) {
    fprintf(stderr, "Usage: %s <server address> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  server_info.addr = argv[1];
  server_info.port = atoi(argv[2]);

  // Create a thread for UI
  pthread_t ui_thread;
  if (pthread_create(&ui_thread, NULL, ui_fn, NULL)) {
    perror("pthread_create_failed");
    exit(EXIT_FAILURE);
  }

  server_loop();

  // Clean up the UI
  // TODO

  return 0;
}

void server_loop() {
  init_server_streams();

  server_pl_t* reply = (server_pl_t*) malloc(sizeof(server_pl_t));
  while (fread(reply, sizeof(server_pl_t), 1, server_info.input) > 0) {
    printf("Client sent: %s\n", reply->msg);
  }

  free(reply);
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
  client_pl_t* pl = (client_pl_t*) malloc(sizeof(client_pl_t));

  while (true) {
    // Get some input from the UI to send to the server
    // Or just general input to handle locally

    strcpy(pl->msg, "Hello!\n");
    send_server_payload(pl);

    // Sleep for one second and then send hello again
    sleep(1);
  }

  free(pl);

  return NULL;
}

void send_server_payload(client_pl_t* pl) {
  if (server_info.output != NULL) {
    fwrite(pl, sizeof(client_pl_t), 1, server_info.output);
    fflush(server_info.output);
  }
}