#ifndef TYPES_H
#define TYPES_H

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

#define MAX_MSG_LEN 256
#define MAX_FILENAME_LEN 32

// Message types between server and client

// All CLIENT_ message types are from client to server
// ---------------------------------------------------

// A debug message type for checking communication between server and client (deprecated)
#define CLIENT_PING 21

// Sent when the local client writes a character to the currently-open file
#define CLIENT_WRITE_CHAR 22

// Sent when the client wants to open a file on the server
#define CLIENT_DOC_REQUEST 23

// All SERVER_ message types are from server to client
// ---------------------------------------------------

// A debug message type to respond to a client's message
#define SERVER_ECHO 11

// Sent when the server indicates it is about to sent a series of
// SERVER_DOC_LINE payloads followed by a SERVER_DOC_END payload
#define SERVER_DOC_START 12

// Sends an individual line of a document 
#define SERVER_DOC_LINE 13

// Sent when the server is done sending a series of lines in the document
#define SERVER_DOC_END 14

// Reply when the server has no file with the name the client asked for
#define SERVER_DOC_NOT_FOUND 15 

// Sent when another client has changed the same file that this client has open
#define SERVER_WRITE_CHAR_RELAY 16

typedef struct server_info {
  char* addr;
  int socket;
  int port;
  FILE* input;
  FILE* output;
  int uid;
  struct server_info* next;
} server_info_t;

typedef struct clients_list {
  server_info_t* head;
} clients_list_t;

typedef struct client_pl {
  char msg[MAX_MSG_LEN];
  int msg_type;
  int x_pos;
  int y_pos;
  char ch;
} client_pl_t;

typedef struct server_pl {
  char msg[MAX_MSG_LEN]; 
  int msg_type;
  int x_pos;
  int y_pos;
  char ch;
} server_pl_t;

typedef struct ui_fn_args {
  char filename[MAX_MSG_LEN];
} ui_fn_args_t;

#endif