#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gap_buffer.h"

// non-NULL, strictly positive max, appropriate start & end
bool is_gap_buff(my_gap_buff* gap_buff){
  return true;
}

// returns wether my_gap_buff is full
bool is_full(my_gap_buff* gap_buff){
  return (gap_buff->start == gap_buff->end);
}

// returns wether the gap is at the buffer's left end
bool is_at_left_end(my_gap_buff* gap_buff){
  return (gap_buff->start == 0);
}

// returns wether the gap is at the buffer's right end
bool is_at_right_end(my_gap_buff* gap_buff){
  return (gap_buff->end == gap_buff->total);
}

// returns wether my_gap_buff is empty
bool is_empty(my_gap_buff* gap_buff){
  return (is_at_left_end(gap_buff) && is_at_right_end(gap_buff));
}

// creates a new gap buffer with max = size
void gap_buff_init(my_gap_buff* gap_buff, int size){

  gap_buff->total = size;
  gap_buff->start = 0;
  gap_buff->end = size;
  gap_buff->buffer = malloc(sizeof(char)*size);

}

// moves the gap to the right (forward)
void gap_buff_forward(my_gap_buff* gap_buff){

  // do we need the first check?
  // what happens if the end is at the very right?
  int gap_length = gap_buff->end - gap_buff->start;
  if (gap_length > 0 && (!is_at_right_end(gap_buff)) ) {
    gap_buff->buffer[gap_buff->start] = gap_buff->buffer[gap_buff->end];
    (gap_buff->start)++;
    (gap_buff->end)++;
  }
  
}

// moves the gap to the left (backward)
void gap_buff_backward(my_gap_buff* gap_buff){

  // do we need the first check?
  int gap_length = gap_buff->end - gap_buff->start;
  if (gap_length > 0 && (!is_at_left_end(gap_buff)) ) {
    gap_buff->buffer[gap_buff->start + gap_buff->end - 1] = gap_buff->buffer[gap_buff->start - 1];
    (gap_buff->start)--;
    (gap_buff->end)--;
  }
  
}

// inserts the character 'c' before the start of the gap
void gap_buff_insert(my_gap_buff* gap_buff, char c){

  // if gap is full double the length and reallocate memory
  if (!gap_buff->end) {
    int length = gap_buff->total - gap_buff->start;
    gap_buff->end = gap_buff->total;
    gap_buff->total = gap_buff->total * 2;
    gap_buff->buffer = realloc(gap_buff->buffer, gap_buff->total);
    memmove(gap_buff->buffer + gap_buff->start + gap_buff->end,
	    gap_buff->buffer + gap_buff->start, length);
  }
  
  // insert the new character and update the struct's fields
  gap_buff->buffer[gap_buff->start] = c;
  (gap_buff->start)++;
}

// delets the character before the gap
void gap_buff_delete(my_gap_buff* gap_buff){

  if ( !(is_at_left_end(gap_buff)) ) {
    (gap_buff->start)--;
        
  }
}


void print_buff(my_gap_buff* gap_buff){
  int length = gap_buff->total;
  for(int i = 0; i < length; i++) {
    printf("%c\n", gap_buff->buffer[i]);
  }
}


int main () {

   
  return 0;
}
