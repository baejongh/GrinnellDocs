#include <stdbool.h>

typedef sturct gap_buffer {

  int max;
  char* buffer;
  int gap_start;
  int gap_end;
  
} my_gap_buff;


// non-NULL, strictly positive max, appropriate start & end
bool is_gap_buff(my_gap_buff* gap_buff);

// returns wether my_gap_buff is empty
bool is_empty(my_gap_buff* gap_buff);

// returns wether my_gap_buff is full
bool is_full(my_gap_buff* gap_buff);

// returns wether the gap is at the buffer's left end
bool is_at_left_end(my_gap_buff* gap_buff);

// returns wether the gap is at the buffer's right end
bool is_at_right_end(my_gap_buff* gap_buff);


// creates a new gap buffer with max = size
my_gap_buff gap_buff_init(my_gap_buff* gap_buff, int size);

// moves the gap to the right (forward)
void gap_buff_forward(my_gap_buff* gap_buff);

// moves the gap to the left (backward)
void gap_buff_backward(my_gap_buff* gap_buff);

// inserts the character 'c' before the start of the gap
void gap_buff_insert(my_gap_buff* gap_buff, char c);

// delets the character before the gap
void gap_buff_delete(my_gap_buff* gap_buff);
