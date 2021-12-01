/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols
 * Modified by <YOUR NAME HERE>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"

#define F_CPU 8000000L
#include <util/delay.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	start_screen();
	
	// Loop forever,
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_terminal_cursor(10,10);
	printf_P(PSTR("Reversi"));
	move_terminal_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 project by Pranay, Student Number: 46444675"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	start_display();
	
	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1) {
		// First check for if a 's' is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's'
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S') {
			break;
		}
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED) {
			break;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	initialise_board();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	
	uint32_t last_flash_time, current_time;
	uint8_t btn; //the button pushed
	uint8_t pause_state = 0;
	uint32_t time_stamp;
	
	update_scores();
	
	
	last_flash_time = get_current_time();
	
	// all ports to output for seven_segment_display	
	DDRC = 0xFF;
	
	// set port A pin 0 to be output
	DDRA = 0x07;
	
	// We play the game until it's over
	while(!is_game_over()) {
				
		// Displays the seven segment display		
		seven_segment_display();
		
		char serial_input1 = -1;
		if(serial_input_available() && pause_state == 1){
			serial_input1 = fgetc(stdin);
			if (serial_input1 == 'P' || serial_input1 == 'p'){
				pause_state = 1 - pause_state;
				current_time = time_stamp;
			}
			if(pause_state == 1){
				continue;
			}	
		}	
		if(pause_state == 0){
			btn = button_pushed();
			char serial_input = -1;
			if(btn == BUTTON0_PUSHED){
				set_square_colour();
			}
			else if(btn == BUTTON1_PUSHED){
				move_display_cursor(0, 1);
			}
			else if(btn == BUTTON2_PUSHED){
				move_display_cursor(-1, 0);
			}
			if (serial_input_available()) {
				// character input from the serial terminal
				serial_input = fgetc(stdin);
				if(serial_input == 'W' || serial_input == 'w'){
					move_display_cursor(0, 1);
				}
				else if(serial_input == 'A' || serial_input == 'a'){
					move_display_cursor(-1, 0);
				}
				else if(serial_input == 'S' || serial_input == 's'){
					move_display_cursor(0, -1);
				}
				else if(serial_input == 'D' || serial_input == 'd'){
					move_display_cursor(1, 0);
				}
				else if (serial_input == ' '){
					set_square_colour();
				}
				if (serial_input == 'P'|| serial_input == 'p'){
					time_stamp = get_current_time();
					init_button_interrupts();
					pause_state = 1 - pause_state;
				}
			}
			
			current_time = get_current_time();
			if(current_time >= last_flash_time + 500) {
				// 500ms (0.5 second) has passed since the last time we
				// flashed the cursor, so flash the cursor
				flash_cursor();
				
				// Update the most recent time the cursor was flashed
				last_flash_time = current_time;
			}
		}
	}
	// We get here if the game is over.
}

void handle_game_over() {
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	
	while(button_pushed() == NO_BUTTON_PUSHED) {
		; // wait
	}
	
}
