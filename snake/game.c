/*
 * game.c
 *
 * Written by Peter Sutton
 */ 

#include <stdio.h>

#include "game.h"
#include "snake.h"
#include "food.h"
#include "superfood.h"
#include "pixel_colour.h"
#include "board.h"
#include "ledmatrix.h"
#include "timer0.h"
#include "rat.h"

// Colours that we'll use
#define SNAKE_HEAD_COLOUR	COLOUR_RED
#define SNAKE_BODY_COLOUR	COLOUR_GREEN
#define FOOD_COLOUR			COLOUR_LIGHT_YELLOW
#define SUPERFOOD_COLOR		COLOUR_ORANGE
#define RAT_COLOUR			COLOUR_LIGHT_GREEN
#define BACKGROUND_COLOUR	COLOUR_BLACK

/* decreasing delay before stepping snake, begins at 600 */
volatile uint16_t move_delay = 600;

// Helper function
static void update_display_at_position(PosnType posn, PixelColour colour) {
	ledmatrix_update_pixel(x_position(posn), y_position(posn), colour);
}

/*
** Super food runtime
*/
void super_food(void) {
	if (get_super_food_status() && get_super_food_existence() == 0) {
		add_super_food();
		update_display_at_position(get_super_food_pos(), SUPERFOOD_COLOR);
	} else if(get_super_food_status() == 0 && get_super_food_existence()) {
		remove_super_food();
		update_display_at_position(get_super_food_pos(), BACKGROUND_COLOUR);
	}
}

void move_rat(void) {
	update_display_at_position(get_rat_pos(), BACKGROUND_COLOUR);
	PosnType newPos = next_rat_pos();
	update_display_at_position(newPos, RAT_COLOUR);
}

// Initialise game. This initialises the board with movsnake and food items 
// and puts them on the display.
void init_game(void) {
	// Clear display
	ledmatrix_clear();
	
	// Initialise the snake and display it. We know the initial snake is only
	// of length two so we can just retrieve the tail and head positions
	init_snake();
	update_display_at_position(get_snake_head_position(), SNAKE_HEAD_COLOUR);
	update_display_at_position(get_snake_tail_position(), SNAKE_BODY_COLOUR);
	
	// Initialise our food store, then add three items of food and display them
	init_food();
	for(int8_t i = 0; i < 3; i++) {
		PosnType food_position = add_food_item();
		if(is_position_valid(food_position)) {
			update_display_at_position(food_position, FOOD_COLOUR);
		}
	}
	add_rat();
	update_display_at_position(get_rat_pos(), RAT_COLOUR);
}

// Attempt to move snake forward. Returns true if successful, false otherwise
int8_t attempt_to_move_snake_forward(void) {
	PosnType prior_head_position = get_snake_head_position();
	int8_t move_result = advance_snake_head();
	if(move_result < 0) {
		// Snake moved out of bounds (if this is not permitted) or
		// collided it with itself. Return false because we couldn't
		// move the snake
		return 0;
	}
	PosnType new_head_position = get_snake_head_position();
	if(move_result == ATE_FOOD || move_result == ATE_SUPER_FOOD
	 || move_result == ATE_FOOD_BUT_CANT_GROW || move_result == ATE_RAT) {
		// reduce step delay
		if(move_delay > 100) {
			move_delay -= 20;
		}
		// remove food item
		if(move_result == ATE_SUPER_FOOD) {
			remove_super_food();
			ate_super_food();
		}  else if(move_result == ATE_RAT) {
			add_rat();
			update_display_at_position(get_rat_pos(), RAT_COLOUR);
		} else {
			int8_t foodID = food_at(new_head_position);
			remove_food(foodID);
			
			// Add a new food item. Might fail if a free position can't be
			// found on the board but shouldn't usually.
			PosnType new_food_posn = add_food_item();
			if(is_position_valid(new_food_posn)) {
				update_display_at_position(new_food_posn, FOOD_COLOUR);
			}
		}
		
		
		// We don't remove the eaten food from the display since we'll just
		// display the snake head at that position.
	}
	
	// If we didn't eat food OR if we ate food but the snake is at 
	// maximum length, then we move the tail forward and remove this 
	// element from the display
	if(move_result == MOVE_OK || move_result == ATE_FOOD_BUT_CANT_GROW) {
		PosnType prev_tail_posn = advance_snake_tail();
		update_display_at_position(prev_tail_posn, BACKGROUND_COLOUR);
	}
	
	// We update the previous head position to become a body part and 
	// update the new head position.
	update_display_at_position(prior_head_position, SNAKE_BODY_COLOUR);
	update_display_at_position(new_head_position, SNAKE_HEAD_COLOUR);
	return 1;
}

/* Gets the current delay before snake moves */
uint16_t get_move_delay(void) {
	return move_delay;
}

/* Reset move delay to 600 */
void init_move_delay(void) {
	move_delay = 600;
}

