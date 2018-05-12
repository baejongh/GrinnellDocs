#ifndef CLIENT_H
#define CLIENT_H

#include "types.h"

void server_loop();
void* ui_fn(void* p);
void init_server_streams();
server_pl_t* send_recieve_msg(client_pl_t* pl);
void send_server_payload(client_pl_t* pl);
void send_client_write_char_payload(int y_pos, int x_pos, char ch);
void payload_handler(server_pl_t* pl);
void ui_display_waiting_for_server();

#endif