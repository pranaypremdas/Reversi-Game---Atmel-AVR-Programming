/*
 * game.c
 *
 * Contains functions relating to the play of the game Reversi
 *
 * Author: Luke Kamols
 */ 

#include <avr/pgmspace.h>
#include <stdio.h>

#include "game.h"
#include "display.h"
#include "terminalio.h"
#include "ledmatrix.h"
#include "serialio.h"
#include "timer0.h"


#define CURSOR_X_START 5
#define CURSOR_Y_START 3

#define START_PIECES 2
static const uint8_t p1_start_pieces[START_PIECES][2] = { {3, 3}, {4, 4} };
static const uint8_t p2_start_pieces[START_PIECES][2] = { {3, 4}, {4, 3} };

uint8_t board[WIDTH][HEIGHT];
uint8_t cursor_x;
uint8_t cursor_y;
uint8_t cursor_visible;
uint8_t current_player;

// Default scores
uint8_t red_score  = 2;
uint8_t green_score = 2;

// Displayed scores
uint8_t player_red_score = 2;
uint8_t player_green_score = 2;

// seven segment array for numbers
uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
	
/* Seven segment display digit being displayed.
** 0 = right digit; 1 = left digit.
*/
volatile uint8_t seven_seg_cc = 0;

/* digits_displayed - 1 if digits are displayed on the seven
** segment display, 0 if not. No digits displayed initially.
*/
volatile uint8_t digits_displayed = 1;

// Determines if the game is paused or not
volatile uint8_t pause_game = 0;

volatile uint32_t time_stamp;



void initialise_board(void) {
	
	// initialize the display we are using
	initialise_display();
	
	// initialize the board to be all empty
	for (uint8_t x = 0; x < WIDTH; x++) {
		for (uint8_t y = 0; y < HEIGHT; y++) {
			board[x][y] = EMPTY_SQUARE;
		}
	}
	
	// now load in the starting pieces for player 1
	for (uint8_t i = 0; i < START_PIECES; i++) {
		uint8_t x = p1_start_pieces[i][0];
		uint8_t y = p1_start_pieces[i][1];
		board[x][y] = PLAYER_1; // place in array
		update_square_colour(x, y, PLAYER_1); // show on board
	}
	
	// and for player 2
	for (uint8_t i = 0; i < START_PIECES; i++) {
		uint8_t x = p2_start_pieces[i][0];
		uint8_t y = p2_start_pieces[i][1];
		board[x][y] = PLAYER_2;
		update_square_colour(x, y, PLAYER_2);		
	}
	
	// set the starting player
	current_player = PLAYER_1;
	
	// also set where the cursor starts
	cursor_x = CURSOR_X_START;
	cursor_y = CURSOR_Y_START;
	cursor_visible = 0;
}

uint8_t get_piece_at(uint8_t x, uint8_t y) {
	// check the bounds, anything outside the bounds
	// will be considered empty
	if (x < 0 || x >= WIDTH || y < 0 || y >= WIDTH) {
		return EMPTY_SQUARE;
	} else {
		//if in the bounds, just index into the array
		return board[x][y];
	}
}

void flash_cursor(void) {
	
	if (cursor_visible) {
		// we need to flash the cursor off, it should be replaced by
		// the colour of the piece which is at that location
		uint8_t piece_at_cursor = get_piece_at(cursor_x, cursor_y);
		update_square_colour(cursor_x, cursor_y, piece_at_cursor);
		
	} else {
		// we need to flash the cursor on
		if(placement_piece_three() == 1){
			update_square_colour(cursor_x, cursor_y, INVALID_MOVE);
		}
		else{
			update_square_colour(cursor_x, cursor_y, CURSOR);
		}
	}
	cursor_visible = 1 - cursor_visible; //alternate between 0 and 1
}

//check the header file game.h for a description of what this function should do
// (it may contain some hints as to how to move the pieces)
void move_display_cursor(uint8_t dx, uint8_t dy) {
	//YOUR CODE HERE
	/*suggestions for implementation:
	 * 1: remove the display of the cursor at the current location
	 *		(and replace it with whatever piece is at that location)
	 * 2: update the positional knowledge of the cursor, this will include
	 *		variables cursor_x, cursor_y and cursor_visible
	 * 3: display the cursor at the new location
	 */
	
	// removing the display of the cursor at the current location
	update_square_colour(cursor_x,cursor_y, get_piece_at(cursor_x, cursor_y));
	
	// Changing the location of the cursor
	cursor_x = (cursor_x + dx) % WIDTH;
	cursor_y = (cursor_y + dy) % HEIGHT;
	
	// flashing the cursor
	flash_cursor();
}

void set_square_colour(void){
	if (get_piece_at(cursor_x, cursor_y) != EMPTY_SQUARE){
		return;
	}
	// Checks for piece placement 3
	if (placement_piece_two() == 1){
		return;
	}
	
	change_led();
	update_square_colour(cursor_x, cursor_y, current_player);
	board[cursor_x][cursor_y] = current_player;
	// if player is player 1 then the square changes to red
	if(current_player == PLAYER_1){
		current_player = PLAYER_2;
	// if player is player 2 then the square changes to green
	}else{
		current_player = PLAYER_1;
	}
	update_scores();
}

void change_led(void){
	PORTA &= 6;
	if(current_player == PLAYER_1){
		PORTA |= 2;
	}
	if(current_player == PLAYER_2){
		PORTA |= 4;
	}
}


void seven_segment_display(void){
	PORTC = 0;	
	seven_seg_cc = 1 - seven_seg_cc;
	if (current_player == PLAYER_1){
		if(seven_seg_cc == 0){
			PORTA = seven_seg_cc;
			PORTA |= 2;
			PORTC = 0;
			if (player_red_score < 10){
				PORTC = seven_seg_data[player_red_score];
			}
			if (player_red_score >= 10){
				PORTC = seven_seg_data[(player_red_score)%10];
			}
		} else if (seven_seg_cc == 1 && player_red_score >= 10){
			PORTC = 0;
			PORTA |= seven_seg_cc;
			PORTC = seven_seg_data[(player_red_score/10)%10];
		}
	}
	// Make this in to an else statement
	else{
		if(seven_seg_cc == 0){
			PORTA = seven_seg_cc;
			PORTA |= 4;
			PORTC = 0;
			if (player_green_score < 10){
				PORTC = seven_seg_data[player_green_score];
			}
			if (player_green_score >= 10){
				PORTC = seven_seg_data[(player_green_score)%10];
			}
		}
		else if (seven_seg_cc == 1 && player_green_score >= 10){
			PORTC = 0;
			PORTA |= seven_seg_cc;
			PORTC = seven_seg_data[(player_green_score/10)%10];
		}
	}
}

void update_scores(void){
	// Local variables
	uint8_t player_1_score = 0;
	uint8_t player_2_score = 0;
	
	//counting the number of reds and greens
	for(uint8_t x_direction = 0; x_direction <= 7; x_direction++){
		for(uint8_t y_direction = 0; y_direction <= 7; y_direction++){
			if(get_piece_at(x_direction, y_direction) == PLAYER_1){
				player_1_score++;
			}
			if(get_piece_at(x_direction, y_direction) == PLAYER_2){
				player_2_score++;
			}
		}
	}
	player_red_score = player_1_score;
	player_green_score = player_2_score;
	hide_cursor();
	move_terminal_cursor(60, 10);
	if(player_red_score <= 9){
		printf_P(PSTR("Red Score:  %3d"), player_red_score);
	}
	else if(player_red_score > 9){
		printf_P(PSTR("Red Score:  %3d"), player_red_score);
	}
	move_terminal_cursor(60, 12);
	if(player_green_score <= 9){
		printf_P(PSTR("Green Score:%3d"), player_green_score);
	}
	else if(player_green_score > 9){
		printf_P(PSTR("Green Score:%3d"), player_green_score);
	}
}


// Checks if flipping is required. If not it retraces back and does nothing
// If flipping is required it changes the color of the LED and then returns the number of points acquired from the move 
int8_t check_if_flipping_required(uint8_t cursor_x, uint8_t cursor_y, int8_t direction_x, int8_t direction_y){
	int8_t check_x_direction = cursor_x + direction_x;
	int8_t check_y_direction = cursor_y + direction_y;
	int8_t points = 0;
	if (get_piece_at(cursor_x, cursor_y) == current_player){
		return 0;
	}
	else if(get_piece_at(cursor_x, cursor_y) == EMPTY_SQUARE){
		return -1;
	}
	else{
		points = check_if_flipping_required(check_x_direction, check_y_direction, direction_x, direction_y);
		if (points == - 1){
			return -1;
		}
		update_square_colour(cursor_x, cursor_y, current_player);
		board[cursor_x][cursor_y] = current_player;
		return points + 1;
	}
}

// Iterates through each possible position except (0,0)
// Checks if flipping is required if so , it updates the score of the player.
// Also does code for Placement 3
uint8_t placement_piece_two(void){
	int8_t points_gained = 0;
	uint8_t initial_points;
	if (current_player == PLAYER_1){
		initial_points = red_score;
	}
	else{
		initial_points = green_score;
	}
	for (int8_t x_direction = -1; x_direction <= 1; x_direction++){ 
		for(int8_t y_direction = -1; y_direction <= 1; y_direction++){
			if (!(x_direction == 0 && y_direction == 0)){
				points_gained = check_if_flipping_required(cursor_x + x_direction, cursor_y + y_direction, x_direction, y_direction);
				if (points_gained != 0 && points_gained != -1){
					if(current_player == PLAYER_1){
						red_score += points_gained;
					}
					else{
						green_score += points_gained;
					}
				}
			}
		}
	}
	// Piece placement 3
	if ((current_player == PLAYER_1 && red_score == initial_points) 
	|| (current_player == PLAYER_2 && green_score == initial_points)){
		update_square_colour(cursor_x, cursor_y, INVALID_MOVE);
		return 1;
	}
	return 0;
}

int8_t check_if_invalid_move(uint8_t cursor_x, uint8_t cursor_y, int8_t direction_x, int8_t direction_y){
	int8_t check_x_direction = cursor_x + direction_x;
	int8_t check_y_direction = cursor_y + direction_y;
	int8_t points = 0;
	if (get_piece_at(cursor_x, cursor_y) == current_player){
		return 0;
	}
	else if(get_piece_at(cursor_x, cursor_y) == EMPTY_SQUARE){
		return -1;
	}
	else{
		points = check_if_invalid_move(check_x_direction, check_y_direction, direction_x, direction_y);
		if (points == - 1){
			return - 1;
		}
		return points + 1;
	}	
}

uint8_t placement_piece_three(){
	int8_t points_gained = 0;
	for (int8_t x_direction = -1; x_direction <= 1; x_direction++){
		for(int8_t y_direction = -1; y_direction <= 1; y_direction++){
			if (!(x_direction == 0 && y_direction == 0)){
				points_gained = check_if_invalid_move(cursor_x + x_direction, cursor_y + y_direction, x_direction, y_direction);
				if (points_gained > 0){
					return 0;
				}
			}
		}
	}	
	return 1;
}
// Call when button B0 or space bar is pressed
// If one is returned there is a valid move possible
// If there is no valid move availbale 0 will be returned
uint8_t placement_piece_four(){
	int8_t points_gained = 0;
	int8_t possible_scoring = 0;
	for(uint8_t x_direction = 0; x_direction <= 7; x_direction++){
		for(uint8_t y_direction = 0; y_direction <= 7; y_direction++){
			for(int8_t dx = -1; dx <= 1; dx++){
				for(int8_t dy = -1; dy <= 1; dy++){
					points_gained = check_if_invalid_move(x_direction + dx, y_direction + dy, dx, dy);
					if (points_gained > 0){
						possible_scoring++;						
					}
						
				}
			}
		}
	}
	if(possible_scoring == 0){
		return 0;
	}
	return 1;
}

uint8_t is_game_over(void) {
	// The game ends when every single square is filled
	// Check for any squares that are empty
	for (uint8_t x = 0; x < WIDTH; x++) {
		for (uint8_t y = 0; y < HEIGHT; y++) {
			if (board[x][y] == EMPTY_SQUARE) {
				// there was an empty square, game is not over
				return 0 ;
			}
		}
	}
	// every single position has been checked and no empty squares were found
	// the game is over
	return 1;
}