/*
 * project.c
 *
 * Main file for the Snake Project.
 *
 * Author: Peter Sutton. Modified by <YOUR NAME HERE>
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>		// For random()

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "board.h"
#include "timer0.h"
#include "game.h"
#include "snake.h"
#include "rat.h"


// Define the CPU clock speed so we can use library delay functions
#define F_CPU 8000000L
#include <util/delay.h>

/* Variables for seven segment display */
volatile uint8_t seven_seg_cc = 0;

/* Seven segment display segment values for 0 to 9 */
uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};

/*
** Seven segment display runtime. Displays length of snake.
*/
void seg_display(void) {
	/* Set bits of port A and most significant bit of port C to be outputs*/
	DDRA = 0x01;
	DDRC = 0xFF;
	
	/* Set up timer for an interrupt every 10 milliseconds. */
	OCR1A = 9999; /* CLK / 8 */
	TCCR1A = 0; /* CTC mode */
	TCCR1B = (1<<WGM12)|(1<<CS11); /* Divide clock by 8 */
	
	TIMSK1 = (1<<OCIE1A); /* Enable interrupt on timer on output compare match */
	TIFR1 = (1<<OCF1A); /* Ensure interrupt flag is cleared */
}

ISR(TIMER1_COMPA_vect) {
	if(get_snake_length() <= 9) {
		PORTA = 0;
		PORTC = seven_seg_data[get_snake_length()];
	} else {
		/* Alternates showing digit */
		seven_seg_cc = 1 ^ seven_seg_cc;
		
		/* Display a digit */
		if((get_snake_length())/10 <= 9 && (get_snake_length())%10 <= 9) {
			if(seven_seg_cc == 0) {
				/* Display right digit*/
				PORTC = seven_seg_data[get_snake_length()%10];
			} else {
				/* Display left digit*/
				PORTC = seven_seg_data[get_snake_length()/10];
			}
		} else {
			if(seven_seg_cc == 0) {
				/* Display right digit */
				PORTC = 0x00;
			} else {
				/* Display left digit */
				PORTC = 0x00;
			}
		}
		PORTA = seven_seg_cc;
	}
}

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);
void handle_new_lap(void);
void terminal_display(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void reset_game(void) {
	while (1){
		new_game();
		play_game();
		handle_game_over();
	}
}

// initialise_hardware()
//
// Sets up all of the hardware devices and then turns on global interrupts
void initialise_hardware(void) {
	// Set up SPI communication with LED matrix
	ledmatrix_setup();
	
	// Set up pin change interrupts on the push-buttons
	init_button_interrupts();
	
	// Initialise joystick interrupts
	//initADC();
	
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	// Set up our main timer to give us an interrupt every millisecond
	init_timer0();
	
	// Turn on global interrupts
	sei();
}

void terminal_display(void) {
	char block = 219;
	for(int8_t x = 4; x < (BOARD_WIDTH+6); x++) {
		move_cursor(x, (BOARD_HEIGHT + 4));
		printf("%c", block);
		
		move_cursor(x, 3);
		printf("%c", block);
		
	}
	for(int8_t y = 4; y < (BOARD_HEIGHT + 5); y++) {
		move_cursor((BOARD_WIDTH + 5), y);
		printf("%c", block);
		
		move_cursor(4, y);
		printf("%c", block);
	}
}

void splash_screen(void) {
	// Reset display attributes and clear terminal screen then output a message
	set_display_attribute(TERM_RESET);
	clear_terminal();
	
	hide_cursor();	// We don't need to see the cursor when we're just doing output
	move_cursor(3,3);
	printf_P(PSTR("Snake"));
	
	move_cursor(3,5);
	set_display_attribute(FG_GREEN);	// Make the text green
	// Modify the following line
	printf_P(PSTR("CSSE2010/7201 Snake Project by Hans Song"));	
	set_display_attribute(FG_WHITE);	// Return to default colour (White)
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	
	// Red message the first time through
	PixelColour colour = COLOUR_RED;
	while(1) {
		set_scrolling_display_text("SNAKE 44374264", colour);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed. We pause for 130ms between each scroll.
		while(scroll_display()) {
			_delay_ms(130);
			if(button_pushed() != -1) {
				// A button has been pushed
				return;
			}
		}
		// Message has scrolled off the display. Change colour
		// to a random colour and scroll again.
		switch(random()%4) {
			case 0: colour = COLOUR_LIGHT_ORANGE; break;
			case 1: colour = COLOUR_RED; break;
			case 2: colour = COLOUR_YELLOW; break;
			case 3: colour = COLOUR_GREEN; break;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	init_game();
		
	// Initialise the score
	init_score();
	
	// Initialise seven segment display
	seg_display();
	
	// Initialize joystick handler
	init_joystick();
	
	// Reset move delay
	init_move_delay();
	
	terminal_display();
	
	// Delete any pending button pushes or serial input
	empty_button_queue();
	clear_serial_input_buffer();
}

void play_game(void) {
	uint32_t last_move_time;
	uint32_t last_rat_move;
	int8_t button;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	
	// Record the last time the snake moved as the current time -
	// this ensures we don't move the snake immediately.
	last_move_time = get_clock_ticks();
	last_rat_move = get_clock_ticks();
	// We play the game forever. If the game is over, we will break out of
	// this loop. The loop checks for events (button pushes, serial input etc.)
	// and on a regular basis will move the snake forward.
	while(1) {
		super_food();
		
		int16_t joystick_x = read_joystick(0);
		int16_t joystick_y = read_joystick(1);
		//printf("x: %u, y: %u\n", joystick_x, joystick_y);
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. We will be processing each character
		// independently and can't do anything until we get the third character.
		// At most one of the following three variables will be set to a value 
		// other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		if(button == -1) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
				// Check if the character is part of an escape sequence
				if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
					// We've hit the first character in an escape sequence (escape)
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
					// We've hit the second character in an escape sequence
					characters_into_escape_sequence++;
					serial_input = -1; // Don't further process this character
				} else if(characters_into_escape_sequence == 2) {
					// Third (and last) character in the escape sequence
					escape_sequence_char = serial_input;
					serial_input = -1;  // Don't further process this character - we
										// deal with it as part of the escape sequence
					characters_into_escape_sequence = 0;
				} else {
					// Character was not part of an escape sequence (or we received
					// an invalid second character in the sequence). We'll process 
					// the data in the serial_input variable.
					characters_into_escape_sequence = 0;
				}
			}
		}
		
		// Process the input. 
		if(button==0 || escape_sequence_char=='C' || joystick_x <= 200) {
			// Set next direction to be moved to be right.
			set_snake_dirn(SNAKE_RIGHT);
		} else  if (button==2 || escape_sequence_char == 'A' || joystick_y >= 800) {
			// Set next direction to be moved to be up
			set_snake_dirn(SNAKE_UP);
		} else if(button==3 || escape_sequence_char=='D' || joystick_x >= 800) {
			// Set next direction to be moved to be left
			set_snake_dirn(SNAKE_LEFT);
		} else if (button==1 || escape_sequence_char == 'B' || joystick_y <= 200) {
			// Set next direction to be moved to be down
			set_snake_dirn(SNAKE_DOWN);
		} else if(serial_input == 'p' || serial_input == 'P') {
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			while ((serial_input = fgetc(stdin))) {
				//move_cursor(3,3);
				//printf("game paused");
				// safeguard to clear any unwanted button presses
				empty_button_queue();
				if (serial_input == 'p' || serial_input == 'P') {
					//move_cursor(3,3);
					//printf("           ");
					break;
				} else if (serial_input == 'n' || serial_input == 'N') {
					reset_game();
				}
			}
		} else if(serial_input == 'n' || serial_input == 'N') {
			reset_game();
		} 
		// else - invalid input or we're part way through an escape sequence -
		// do nothing		
		
		if(get_clock_ticks() >= last_rat_move + 1000) {
			move_rat();
			last_rat_move = get_clock_ticks();
		}
		
		// Check for timer related events here
		if(get_clock_ticks() >= last_move_time + get_move_delay()) {
			// move_delay seconds has passed since the last time we moved the snake (default 600),
			// so move it now
			if(!attempt_to_move_snake_forward()) {
				// Move attempt failed - game over
				break;
			}
			last_move_time = get_clock_ticks();
		}
	}
	// If we get here the game is over. 
}

void handle_game_over() {
	move_cursor(10,14);
	// Print a message to the terminal. 
	printf_P(PSTR("GAME OVER"));
	move_cursor(10,15);
	printf_P(PSTR("Press a button to start again"));
	while(button_pushed() == -1) {
		char serial_input = fgetc(stdin);
		if (serial_input == 'n' || serial_input == 'N') {
			reset_game();
		}
		; // wait until a button has been pushed
	}
	
}
