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

#define DEFAULT_PORT 4444

clients_list_t* connected_clients;
pthread_mutex_t clients_lock;

typedef struct thread_arg {
  int socket_fd;
  int client_number;
} thread_arg_t;

void* client_thread_fn(void* p);
void client_doc_request_handler(client_pl_t* pl, FILE* reply_stream);
void send_client_payload(server_pl_t* pl, FILE* reply_stream);
int  send_client_doc_lines(server_pl_t* reply, FILE* reply_stream);
void send_client_doc_end_reply(server_pl_t* reply, FILE* reply_stream);
void send_client_doc_start_reply(server_pl_t* reply, FILE* reply_stream);
void send_empty_typed_reply(server_pl_t* reply, FILE* reply_stream, int reply_type);
void client_ping_handler(client_pl_t* pl, FILE* reply_stream);
void client_write_char_handler(client_pl_t* pl, FILE* reply_stream);
void payload_handler(client_pl_t* pl, FILE* reply_stream);
void compute_offset(FILE* file, int x, int y);
void server_file_update (FILE* file, client_pl_t* pl);
void relay_file_update(server_pl_t* reply, FILE* reply_stream);

void add_client(server_info_t* new_client);
void remove_client(int uid_to_remove);

int main(int argc, char** argv) {
  int port = argc == 2 ? atoi(argv[1]) : DEFAULT_PORT;

  // init the clients list
  connected_clients = (clients_list_t*) malloc (sizeof(clients_list_t));

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
    server_info_t* new_client = (server_info_t*) malloc(sizeof(server_info_t));
    new_client->uid = client_count;
    new_client->socket = client_socket;
    add_client(new_client);
    
    // Set up arguments for the client thread
    thread_arg_t* args = malloc(sizeof(thread_arg_t));
    args->socket_fd = client_socket;
    args->client_number = client_count;
    
    // Create the thread
    pthread_t thread;
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
  
  // Duplicate the socket_fd so we can open it twice, once for input and once for output
  int socket_fd_copy = dup(socket_fd);
  if(socket_fd_copy == -1) {
    perror("dup failed");
    exit(EXIT_FAILURE);
  }
  
  // Open the socket as a FILE stream so we can use fgets
  FILE* input  = fdopen(socket_fd, "r");
  FILE* output = fdopen(socket_fd_copy, "w");
  
  // update client's input/output streams
  /*
  server_info_t* cur_client = connected_clients->head;
  while (cur_client != NULL) {
    if (cur_client->uid == client_number) {
      cur_client->input = input;
      cur_client->output = output;
    }
  }*/

  // Check for errors
  if(input == NULL || output == NULL) {
    perror("fdopen failed");
    exit(EXIT_FAILURE);
  }
  
  // Read lines until we hit the end of the input (the client disconnects)
  char* line = NULL;
  size_t linecap = 0;
  client_pl_t* pl = (client_pl_t*) malloc(sizeof(client_pl_t));
  // server_pl_t* reply = (server_pl_t*) malloc(sizeof(server_pl_t));
  // reply->msg_type = SERVER_ECHO;
  
  while(fread(pl, sizeof(client_pl_t), 1, input)) {
    payload_handler(pl, output);

    // Print a message on the server
    // 
    
    // // Send the message to the client. Flush the buffer so the message is actually sent.
    // fwrite(reply, sizeof(server_pl_t), 1, output);
    // strcpy(reply->msg, pl->msg);
    // fflush(output);
  }
  
  // When we're done, we should free the line from getline
  free(line);
  free(pl);
  
  // Print information on the server side
  printf("Client %d disconnected.\n", client_number);
  
  // TODO: FIX ISSUE!!
  //remove_client(client_number);

  return NULL;
}

void payload_handler(client_pl_t* pl, FILE* reply_stream) {
  switch(pl->msg_type) {
    case CLIENT_PING:
      client_ping_handler(pl, reply_stream);
      break;
    case CLIENT_WRITE_CHAR:
      client_write_char_handler(pl, reply_stream);
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
  int ret = send_client_doc_lines(reply, reply_stream);
  if (ret == -1) {
    fprintf(stderr, "Failed to open file with name: %s\n", pl->msg);
  }

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

// returns:
//  on success: 0
//  on failure: -1
int send_client_doc_lines(server_pl_t* reply, FILE* reply_stream) {
  // Open file
  FILE* file = fopen("alek.txt", "r");
  if (file == NULL) {
    return -1;
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

  return 0;
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
  // STUB
  printf("Client pinged!\n");
}

void client_write_char_handler(client_pl_t* pl, FILE* reply_stream) {
  // STUB
  printf("Client wants to write: %c at position (x, y): (%d, %d)\n", 
    pl->ch, pl->x_pos, pl->y_pos);

  // open the file to write the user updates to
  char* filename = "alek.txt";
  FILE* file = fopen(filename, "r+");

  server_file_update(file, pl);

  server_pl_t* reply = (server_pl_t*) malloc(sizeof(server_pl_t));
  reply->x_pos = pl->x_pos;
  reply->y_pos = pl->y_pos;
  reply->ch = pl->ch;

  // loop through the list of connected clients and send them the updated character
  server_info_t* cur_client = connected_clients->head;

  //while(cur_client != NULL){

    //TODO: Check client's id so it does not send the update back to the same user
    //relay_file_update(reply, reply_stream);
  //}

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

  fclose(file);
}
 

void relay_file_update(server_pl_t* reply, FILE* reply_stream) {

  reply->msg_type = SERVER_WRITE_CHAR_RELAY;
  send_client_payload(reply, reply_stream);
  
}


void add_client(server_info_t* new_client) {
  
  new_client->next = connected_clients->head;

}

void remove_client(int uid_to_remove) {
  pthread_mutex_lock(&clients_lock);

  if(connected_clients->head == NULL) {
    printf("No clients are connected\n");
  }
  else {
    server_info_t* current_client = connected_clients->head;
    server_info_t* previous_client = NULL;
    int cur_uid;

    printf("Before the if\n");
    // if we want to remove the head
    if (current_client->uid == uid_to_remove) {
            printf("In the if \n");

      connected_clients->head = current_client->next;
    }
    else {
      printf("In the else\n");
      // iterate through the list of connected clients
      while(current_client != NULL) {
        cur_uid = current_client->uid;

        // if uids match, remove the client
        if (cur_uid == uid_to_remove) {
          previous_client->next = current_client->next;
          break;
        }

        // advance the pointers
        current_client = current_client->next;
        previous_client = current_client;
      }
    }

    pthread_mutex_unlock(&clients_lock);
  }
}
