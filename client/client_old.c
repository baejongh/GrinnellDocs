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

#define MAX_MSG_LEN 512

typedef struct thread_arg {
  int socket_fd;
  int client_number;
  bool isParentConn;
} thread_arg_t;

typedef struct local_client_fn_arg {
  char* local_user;
} local_client_fn_arg_t;

typedef struct server_info {
  char* addr;
  int socket;
  int port;
} server_info_t;

typedef struct client_pl {
  char msg[MAX_MSG_LEN];
} client_pl_t;

typedef struct server_pl {
  // Obviously, put something different here, e.g. success code or smth.
  char msg[MAX_MSG_LEN]; 
} server_pl_t;

server_pl_t* send_recieve_msg(client_pl_t* pl);

server_info_t server_info;

int main(int argc, char** argv) {
  if(argc != 4) {
    fprintf(stderr, "Usage: %s <server address> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  server_info.addr = argv[2];
  server_info.port = atoi(argv[3]);

  // Connect to server
  // TODO

  ui_add_message(NULL, "Starting server loop\n");

  // Create a thread for UI
  // TODO

  // Clean up the UI
  // TODO

  return 0;
}

// Sends a to_server_pl_t to the server and blocks until a 
// to_client_pl_t is recieved.
server_pl_t* send_recieve_msg(client_pl_t* payload) {
  // Stub. Send the server a message!
}