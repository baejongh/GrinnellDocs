#ifndef UI_H
#define UI_H

/**
 * Initialize the chat user interface. Call this once at startup.
 */
void setup_window();

/**
 * 
 */
void ui_append_char(char c);

/**
 * 
 */
void user_actions(int n);

/**
 * 
 */
void ui_write_file(char* filename);

void ui_write_line(char* line);

void ui_init(char* filename);

void ui_display_waiting_for_server();

#endif