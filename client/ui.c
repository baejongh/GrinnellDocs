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

char* messages[CHAT_HEIGHT];
size_t text_lenght = 0;


void ui_init() {
  // Create the main window
  mainwin = initscr();
  if(mainwin == NULL) {
    fprintf(stderr, "Failed to initialize screen\n");
    exit(EXIT_FAILURE);
  }
  
  // initialize mode, x, y
  mode = 'n'
  x=0;
  y=0;

  // Don't display characters when they're pressed
  noecho();

  // Initialize ncurses
  initscr();
  // disable the buffering of typed characters by the TTY driver and get character at a time input
  cbreak();
  // get keys like backspace, delete and four arrow keys by getch()
  keypad(stdscr,TRUE);
  
  // Create the text editor window
  textwin = subwin(mainwin, TEXT_HEIGHT + 2, WIDTH + 2, 0, 0);
  box(textwin, 0, 0);
  
  
}

void write_message(char* c) {
  // display message
  mvwaddstr(textwin, y, x, c);
  wrefresh(textwin);
  // increment x and y
  x++;
  if (x == 99) {
    y++;
    x = 0;
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
        if (x + 1 <= 99) {
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
        if (y + 1 <= 99) {
          y++;
          move(y,x);
        }
        return;
    }
    switch(mode)
    {
    case 'n':
        switch(c)
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
        switch(c)
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
                write_message("");
                // sets the curser at the correct location
                x = x - 2;
                move(y,x);
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
                write_message("");
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
            write_message("  ")
            x;
            break;
        default:
            // Any other character
            write_message(c);
            x++;
            break;
        }
        break;
    }
}

int main(int argc, char* argv[])
{
    string fn = "";
    if(argc > 1)
    {
        fn = argv[1];               // Set the filename
    }

    // initialize the ui
    ui_init();

    // read in the file
    File *file;
    char buffer[1024];
    size_t c;
    file = fopen(fn, "r");
    while(1) {
      c = fgetc(file);
      if (feof(file)) {
        break;
      }
      write_message(c);
    }
    fclose(fp);

    while(1) {
      int input = getch();

    }




    refresh();                      // Refresh display
    endwin();                       // End ncurses mode
    return 0;
}
