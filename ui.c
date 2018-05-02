#include "ui.h"

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 100
#define TEXT_HEIGHT 100
#define USERNAME_DISPLAY_MAX 8

WINDOW* mainwin;
WINDOW* textwin;
WINDOW* inputwin;

// for the ui x position, y position, and mode
int x, y;
char mode;

char* messages[TEXT_HEIGHT];

void setup_window() {
  // Create the main window
  mainwin = initscr();
  if(mainwin == NULL) {
    fprintf(stderr, "Failed to initialize screen\n");
    exit(EXIT_FAILURE);
  }
  
  // initialize mode, x, y
  mode = 'n';
  x = 0;
  y = 0;

  // Don't display characters when they're pressed
  noecho();

  // Initialize ncurses
  initscr();
  // disable the buffering of typed characters by the TTY driver and get character at a time input
  cbreak();
  // get keys like backspace, delete and four arrow keys by getch()
  keypad(stdscr,TRUE);

  refresh();
}

void write_message(char c) {
    if (c == '\n') {
        y++;
        x = 0;
        move(y,x);
    }
    else {
        // display message
        mvwaddch(mainwin, y, x, c);
        wrefresh(mainwin);
        // increment x and y
        x++;
        if (x > WIDTH) {
            y++;
            x = 0;
        }
    }
}

void user_actions(int n) {

    switch(n) {
    case KEY_LEFT:
        if (x - 1 >= 0) {
          x--;
          move(y,x);
        }
        return;
    case KEY_RIGHT:
        if (x + 1 < WIDTH) {
          x++;
          move(y,x);
        }
        return;
    case KEY_UP:
        if (y - 1 >= 0) {
          y--;
          move(y,x);
        }
        return;
    case KEY_DOWN:
        if (y + 1 < TEXT_HEIGHT) {
          y++;
          move(y,x);
        }
        return;
    }
    switch(mode)
    {
    case 'n':
        switch(n)
        {
        case 'x':
            // Press 'x' to exit
            mode = 'x';
            break;
        case 'i':
            // Press 'i' to enter insert mode
            mode = 'i';
            break;
        case 's':
            // Press 's' to save the current file
            break;
        }
        break;
    case 'i':
        switch(n)
        {
        case 27:
            // The Escape/Alt key
            mode = 'n';
            break;
        case 127:
        case KEY_BACKSPACE:
            // The Backspace key
            if(x == 0)
            {
                // do nothing
                // need to move up the line

            }
            else
            {
                // Removes a character
                mvwaddch(mainwin, y, x, ' ');
                // sets the curser at the correct location
                x = x - 1;
                move(y,x);
                wrefresh(mainwin);

            }
            break;
        case KEY_DC:
            // The Delete key
            if(x == 99)
            {
                // do nothing for now, need to move the line down
            }
            else
            {
                // Removes a character
                
                mvwaddch(mainwin, y, x, ' ');
                x--;
                move(y,x);

            }
            break;
        case KEY_ENTER:
        case 10:
            // The Enter key
            // Bring the rest of the line down
            if(y < 100)
            {
                // Put the rest of the line on a new line
                y++;
                x = 0;
            }
            else
            {
                // do nothing
            }
            move(y,x);
            break;
        case KEY_BTAB:
        case KEY_CTAB:
        case KEY_STAB:
        case KEY_CATAB:
        case 9:
            // The Tab key
            write_message('\t');
            x = x + 2;
            move(y,x);
            break;
        default:
            // Any other character
            write_message((char) n);
            move(y,x);

            break;
        }
        break;
    }
}

void ui_init(char* filename)
{
    if (filename == NULL) {
        fprintf(stderr, "A valid filename is required: current filename is: %s", filename);
    }          

    // Initialize UI window
    setup_window();
    write_file_to_ui(filename);

    // UI user input loop
    while(1) {
      int input = getch();
      user_actions(input);
    }

    refresh(); // Refresh display
    endwin();  // End ncurses mode
}

void write_file_to_ui(char* filename) {
    // read in the file
    FILE *file;
    size_t c;

    file = fopen(filename, "r");

    while(1) {
        int c = fgetc(file);
        if (feof(file) || c == EOF) {
            break;
        }
        write_message(c);
    }

    fclose(file);
}
