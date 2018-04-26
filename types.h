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

typedef struct server_info {
  char* addr;
  int socket;
  int port;
  FILE* input;
  FILE* output;
} server_info_t;

typedef struct client_pl {
  char msg[MAX_MSG_LEN];
} client_pl_t;

typedef struct server_pl {
  // Obviously, put something different here, e.g. success code or smth.
  char msg[MAX_MSG_LEN]; 
} server_pl_t;