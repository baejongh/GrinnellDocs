#ifndef UI_H
#define UI_H

#define WIDTH 100
#define TEXT_HEIGHT 100
#define USERNAME_DISPLAY_MAX 8

/**
 * Initialize the chat user interface. Call this once at startup.
 */
void setup_window();

/**
 * 
 */
void ui_append_char(char c);

void ui_append_char_(char c);

void ui_place_char(char c, int x, int y);

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

void ui_init_window();

#endif