#ifndef UI_H
#define UI_H

/**
 * Initialize the chat user interface. Call this once at startup.
 */
void setup_window();

/**
 * 
 */
void write_message(char c);

/**
 * 
 */
void user_actions(int n);

/**
 * 
 */
void write_file_to_ui(char* filename);

#endif