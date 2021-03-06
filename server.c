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
#include "server.h"

#define DEFAULT_PORT 4444

pthread_mutex_t clients_lock;
node_t* client_lst = NULL;

int main(int argc, char** argv) {
  int port = argc == 2 ? atoi(argv[1]) : DEFAULT_PORT;
  
  // init locks
  pthread_mutex_init(&clients_lock, NULL);

  // Set up a socket
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s == -1) {
    perror("socket");
    exit(2);
  }

  // Listen at this address. We'll bind to port DEFAULT_PORT to sendaccept any available port or use the port specified
  struct sockaddr_in addr = {
    .sin_addr.s_addr = INADDR_ANY,
    .sin_family = AF_INET,
    .sin_port = htons(port)
  };

  // Bind to the specified address
  if(bind(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
    perror("bind");
    exit(2);
  }

  // Become a server socket
  listen(s, 2);
  
  // Get the listening socket info so we can find out which port we're using
  socklen_t addr_size = sizeof(struct sockaddr_in);
  getsockname(s, (struct sockaddr *) &addr, &addr_size);
  
  // Print the port information
  printf("Listening on port %d\n", ntohs(addr.sin_port));
  
  int client_count = 0;

  // Repeatedly accept connections
  while(true) {
    // Accept a client connection
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);
    int client_socket = accept(s, (struct sockaddr*)&client_addr, &client_addr_len);
    
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ipstr, INET_ADDRSTRLEN);
    
    printf("Client %d connected from %s\n", client_count, ipstr);

    // Set up arguments for the client thread
    thread_arg_t* args = malloc(sizeof(thread_arg_t));
    args->socket_fd = client_socket;
    args->client_number = client_count;
    
    // Create the thread
    pthread_t thread;
    printf("about to create thread\n");
    if(pthread_create(&thread, NULL, client_thread_fn, args)) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
    
    client_count++;
  }
  close(s);
}

void* client_thread_fn(void* p) {
  // Unpack the thread arguments
  thread_arg_t* args = (thread_arg_t*)p;
  int socket_fd = args->socket_fd;
  int client_number = args->client_number;
  free(args);

  printf("client_thread_fn: client %d\n", client_number);
  
  // Duplicate the socket_fd so we can open it twice, once for input and once for output
  int socket_fd_copy = dup(socket_fd);
  if(socket_fd_copy == -1) {
    perror("dup failed");
    exit(EXIT_FAILURE);
  }
  
  // Open the socket as a FILE stream so we can use fgets
  FILE* input  = fdopen(socket_fd, "r");
  FILE* output = fdopen(socket_fd_copy, "w");
  add_conn(client_number, output);

  // Check for errors
  if(input == NULL || output == NULL) {
    perror("fdopen failed");
    exit(EXIT_FAILURE);
  }
  
  // Read lines until we hit the end of the input (the client disconnects)
  char* line = NULL;
  size_t linecap = 0;
  client_pl_t* pl = (client_pl_t*) malloc(sizeof(client_pl_t));
  
  while(fread(pl, sizeof(client_pl_t), 1, input) > 0) {
    payload_handler(pl, output, client_number);
  }
  
  // When we're done, we should free the line from getline
  free(line);
  free(pl);
  
  // Print information on the server side
  printf("Client %d disconnected.\n", client_number);
  remove_conn(client_number);

  return NULL;
}

void payload_handler(client_pl_t* pl, FILE* reply_stream, int client_id) {
  printf("Recieved payload of type %d from client %d\n", pl->msg_type, client_id);
  switch(pl->msg_type) {
    case CLIENT_PING:
      client_ping_handler(pl, reply_stream);
      break;
    case CLIENT_WRITE_CHAR:
      client_write_char_handler(pl, reply_stream, client_id);
      break;
    case CLIENT_DOC_REQUEST:
      client_doc_request_handler(pl, reply_stream);
      break;
    default:
      perror("Unexpected message type from client");
      break;
  }
}

// Assumes pl->msg_type == CLIENT_DOC_REQUEST
void client_doc_request_handler(client_pl_t* pl, FILE* reply_stream) {
  server_pl_t* reply = (server_pl_t*) malloc(sizeof(server_pl_t));

  send_client_doc_start_reply(reply, reply_stream);
  send_client_doc_lines(reply, pl->msg, reply_stream);
  send_client_doc_end_reply(reply, reply_stream);

  free(reply);
}

void send_empty_typed_reply(server_pl_t* reply, FILE* reply_stream, int reply_type) {
  reply->msg_type = reply_type;
  send_client_payload(reply, reply_stream);
}

void send_client_doc_start_reply(server_pl_t* reply, FILE* reply_stream) {
  send_empty_typed_reply(reply, reply_stream, SERVER_DOC_START);
}

void send_client_doc_end_reply(server_pl_t* reply, FILE* reply_stream) {
  send_empty_typed_reply(reply, reply_stream, SERVER_DOC_END);
}

void send_client_doc_not_found_reply(server_pl_t* reply, FILE* reply_stream) {
  send_empty_typed_reply(reply, reply_stream, SERVER_DOC_NOT_FOUND);
}

// returns:
//  on success: 0
//  on failure: -1
void send_client_doc_lines(server_pl_t* reply, char* filename, FILE* reply_stream) {
  // Open file
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    printf("Creating file: %s\n", filename);
    file = fopen(filename, "ab+");
    send_client_doc_not_found_reply(reply, reply_stream);
    fclose(file);
    return;
  }

  reply->msg_type = SERVER_DOC_LINE;

  size_t counter = 0;
  int ch;
  while (!feof(file) || ch != EOF) {
    // Copy contents of file into payload
    ch = fgetc(file);
    reply->msg[counter] = ch;
    counter++;

    // If payload msg field is full, send the payload
    if (counter == MAX_MSG_LEN) {
      send_client_payload(reply, reply_stream);
      counter = 0;
    }
  }

  // Send any remaining characters to the client
  if (counter > 0) {
    reply->msg[--counter] = '\0';
    send_client_payload(reply, reply_stream);
  }

  fclose(file);
}

// pl: the payload to send to the client
// reply_stream: the stream onto which we can write our reply to the client
void send_client_payload(server_pl_t* pl, FILE* reply_stream) {
  printf("Sending payload type: %d\n", pl->msg_type);

  if (reply_stream != NULL) {
    fwrite(pl, sizeof(client_pl_t), 1, reply_stream);
    fflush(reply_stream);
  } else {
    perror("File stream to respond to client is somehow NULL");
  }
}

void client_ping_handler(client_pl_t* pl, FILE* reply_stream) {
  printf("Client pinged!\n");
}

void client_write_char_handler(client_pl_t* pl, FILE* reply_stream, int this_client_id) {
  printf("Client wants to write: %c at position (x, y): (%d, %d)\n", 
    pl->ch, pl->x_pos, pl->y_pos);

  // open the file to write the user updates to
  printf("pl->msg: %s\n", pl->msg);
  FILE* file = fopen(pl->msg, "r+");

  server_file_update(file, pl);
  server_pl_t* reply = (server_pl_t*) malloc(sizeof(server_pl_t));
  broadcast_write_char(reply, pl, this_client_id);
  free(reply);

  fclose(file);
}

void compute_offset(FILE* file, int x, int y) {

  size_t ch;

  int cur_x = 0;
  int cur_y = 0;

  while (!feof(file) || ch != EOF) {

    while (cur_y < y) {

      ch = getc(file);

      if (ch == '\n' || cur_x == WIDTH) {
        cur_y ++;
        cur_x = 0;
      }
      else {
        cur_x ++;
      }
    }

    while (cur_x < x) {
      ch = fgetc(file);
      cur_x++;
    }

    break;
  }
}

void server_file_update (FILE* file, client_pl_t* pl) {

  int x_update = pl->x_pos;
  int y_update = pl->y_pos;

  compute_offset(file, x_update, y_update);

  char ret = fputc(pl->ch, file);
}

void add_conn(int client_id, FILE* output) {
  pthread_mutex_lock(&clients_lock);
  node_t* n = (node_t*) malloc(sizeof(node_t));
  n->client_id = client_id;
  n->output = output;
  n->next = NULL;

  if (client_lst == NULL) {
    client_lst = n;
  } else {
    node_t* temp = client_lst;
    client_lst = n;
    n->next = temp;
  }
  pthread_mutex_unlock(&clients_lock);
}

void remove_conn(int client_id) {
  pthread_mutex_lock(&clients_lock);
  if (client_lst == NULL) {
    perror("Tried to remove client from client list with empty list");
    pthread_mutex_unlock(&clients_lock);
    return;
  }

  node_t* fst = client_lst;
  if (fst->client_id == client_id) {
    client_lst = fst->next;
    free(fst);
    pthread_mutex_unlock(&clients_lock);
    return;
  }

  node_t* prev = fst;
  node_t* cur  = fst->next;
  while (cur->client_id != client_id && cur != NULL) {
    prev = prev->next;
    cur = cur->next;
  }

  prev->next = cur->next;
  free(cur);
  pthread_mutex_unlock(&clients_lock);
}

void broadcast_write_char(server_pl_t* reply, client_pl_t* pl, int this_client_id) {
  reply->msg_type = SERVER_WRITE_CHAR_RELAY;
  reply->x_pos = pl->x_pos;
  reply->y_pos = pl->y_pos;
  reply->ch = pl->ch;

  node_t* cur = client_lst;
  while (cur != NULL) {
    if (cur->client_id != this_client_id) {
      send_client_payload(reply, cur->output);
    }
    cur = cur->next;
  }
}