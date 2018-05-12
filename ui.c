#include "ui.h"
#include "client.h"

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 100
#define TEXT_HEIGHT 100
#define USERNAME_DISPLAY_MAX 8

char input[100];

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

// Appends a char to the UI without sending a payload to the server
void ui_append_char_(char c) {
    if (c == '\n') {
        y++;
        x = 0;
        move(y,x);
    }
    else {
        // display message
        mvwaddch(mainwin, y, x, c);
        // increment x and y
        x++;
        if (x == WIDTH) {
            y++;
            x = 0;
        }
    }  
}

void ui_append_char(char c) {
    send_client_write_char_payload(y, x, c);
    ui_append_char_(c);
}

void space(int y, int x) {
    char* file[100];
    // initialize the 2d array
    for (int i = 0; i < WIDTH; i++) {
        file[i] = (char*) malloc(WIDTH * sizeof(char));
    }

    // grab the file
    int height = y;
    while (height < WIDTH) {
        // for the first line
        if (height == y) {
            for (int i = x; i < WIDTH; i++) {
                file[height][i] = (char) mvwinch(mainwin, height, i);
            }
        }
        else {
            for (int i = 0; i < WIDTH; i++) {
                file[height][i] = (char) mvwinch(mainwin, height, i);
            }
        }
        height++;
    }

    // write space
    ui_append_char(' ');

    // populate rest of the ui
    int hold = x;
    int y_hold = y;
    height = y;
    while (height < WIDTH) {
        // for the first line
        if (height == y) {
            for (int i = hold; i < WIDTH; i++) {
                ui_append_char_(file[height][i]);
            }
        }
        else {
            for (int i = 0; i < WIDTH; i++) {
                ui_append_char_(file[height][i]);
            }
        }
        height++;
    }

    for (int i = 0; i < WIDTH; i++) {
        free(file[i]);
    }

    x = hold;
    y = y_hold;
}

void enter(int y, int x) {
    char* file[100];
    // initialize the 2d array
    for (int i = 0; i < WIDTH; i++) {
        file[i] = (char*) malloc(WIDTH * sizeof(char));
    }

    int count = 0;
    // populate the line we want to enter
    for (int i = x; i < WIDTH; i++) {
        file[y][count] = (char) mvwinch(mainwin, y, i);
        mvwaddch(mainwin, y, i, ' ');
        count++;
    }

    for (int i = count; i < WIDTH; i++) {
        file[y][i] = ' '; //?
    }

    // grab the rest of the file
    int height = y+1;
    while (height < WIDTH) {
        for (int i = 0; i < WIDTH; i++) {
            file[height][i] = (char) mvwinch(mainwin, height, i);
        }
        height++;
    }

    for (int i = y; i < WIDTH; i++) {
        for (int j = 0; j < WIDTH; j++) {
            mvwaddch(mainwin, i+1, j, file[i][j]);
        }
    }

    for (int i = 0; i < WIDTH; i++) {
        free(file[i]);
    }

    wrefresh(mainwin);
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
            exit(1);
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
            if(x < 0)
            {
                // do nothing
                // need to move up the line

            }
            else
            {
                // Removes a character
                mvwaddch(mainwin, y, x, ' ');
                // sets the curser at the correct location
                if (x == 0) {
                    x = 0;
                } else {
                    x = x - 1;
                }
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
            if(x < 100)
            {
                enter(y,x);
                x = 0;
                y++;
                // Put the rest of the line on a new line
                move(y,x);
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
            // The Tab key default is 2 space
            ui_append_char((char) '\t');
            x = x + 2;
            move(y,x);
            break;
        default:
            // Any other character=
            ui_append_char((char) n);
            move(y,x);
            wrefresh(mainwin);

            break;
        }
        break;
    }
}

// Deprecated (no longer persist files on the client-side)
void ui_init(char* filename)
{
    if (filename == NULL) {
        fprintf(stderr, "A valid filename is required: current filename is: %s", filename);
    }

    // Initialize UI window
    setup_window();
    ui_write_file(filename);

    // UI user input loop
    while(true) {
      int input = getch();
      user_actions(input);
    }

    refresh(); // Refresh display
    endwin();  // End ncurses mode
}

void ui_init_window() {
    // Initialize UI window
    setup_window();

    while(true) {
      int input = getch();
      user_actions(input);
    }

    refresh(); // Refresh display
    endwin();  // End ncurses mode
}

void ui_write_file(char* filename) {
    // read in the file
    FILE *file;
    size_t c;

    file = fopen(filename, "r");

    while(!feof(file) || c != EOF) {
        int c = fgetc(file);
        ui_append_char(c);
    }

    fclose(file);
}

void ui_write_line(char* line) {
    __uint8_t* char_ptr = (__uint8_t*) line;
    int counter = 0;
    while (char_ptr != NULL && counter < MAX_MSG_LEN) {
        ui_append_char_((char) *char_ptr);
        char_ptr++;
        counter++;
    }
}

void ui_display_waiting_for_server() {
    // STUB
}
