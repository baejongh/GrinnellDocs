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

#define SERVER_ECHO 1
#define CLIENT_PING 2
#define CLIENT_WRITE_CHAR 3

typedef struct server_info {
  char* addr;
  int socket;
  int port;
  FILE* input;
  FILE* output;
} server_info_t;

typedef struct client_pl {
  char msg[MAX_MSG_LEN];
  int msg_type;
  int x_pos;
  int y_pos;
  char ch;
} client_pl_t;

typedef struct server_pl {
  // Obviously, put something different here, e.g. success code or smth.
  char msg[MAX_MSG_LEN]; 
  int msg_type;
} server_pl_t;

typedef struct ui_fn_args {
  char filename[MAX_MSG_LEN];
} ui_fn_args_t;

#endif