/*
** game.h
**
** Written by Luke Kamols
**
** Function prototypes for those functions available externally
*/

#ifndef GAME_H_
#define GAME_H_

#include <inttypes.h>

// initialise the display of the board, this creates the internal board
// and also updates the display of the board
void initialise_board(void);

// returns which piece is located at position (x,y)
// the value returned will be SQUARE_EMPTY, SQUARE_P1 or SQUARE_P2
// anything outside the bounds of the boards will be SQUARE_EMPTY
uint8_t get_piece_at(uint8_t x, uint8_t y);

// update the cursor display, by changing whether it is visible or not
// call this function at regular intervals to have the cursor flash
void flash_cursor(void);

// moves the position of the cursor by (dx, dy) such that if the cursor
// started at (cursor_x, cursor_y) then after this function is called, 
// it should end at ( (cursor_x + dx) % WIDTH, (cursor_y + dy) % HEIGHT)
// the cursor should be displayed after it is moved as well
void move_display_cursor(uint8_t dx, uint8_t dy);

// returns 1 if the game is over, 0 otherwise
uint8_t is_game_over(void);

// sets the place selected by the player
void set_square_colour(void);

// Updates the score of the players
void update_scores(void);

// Displays the scores of the player
void seven_segment_display(void);

// shows which players turn it is on the IO Board
void change_led(void);

// Codes for placement piece 2
uint8_t placement_piece_two(void);

// Codes for piece placement 3
uint8_t placement_piece_three(void);

// Codes for piece placement 4
uint8_t placement_piece_four(void);

#endif

