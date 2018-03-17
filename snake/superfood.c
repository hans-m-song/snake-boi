/*
** food.c
**
** Written by Peter Sutton edited by Hans Song
**
*/

#include <stdlib.h>

#include "position.h"
#include "superfood.h"
#include "rat.h"
#include "food.h"
#include "snake.h"
#include "board.h"
#include "timer0.h"

uint8_t super_food_exists;

PosnType super_food_pos;

PosnType add_super_food(void) {
	int8_t x, y, attempts;
	PosnType test_position;
	x = 0;
	y = 0;
	attempts = 0;
	srandom(get_clock_ticks());
	do {
		// Generate a new position - this is based on a sequence rather
		// then being random
		x = random()%BOARD_WIDTH;
		y = random()%BOARD_HEIGHT;
		test_position = position(x,y);
		attempts++;
	} while(attempts < 100 && 
				(is_snake_at(test_position) || is_food_at(test_position) ||
				is_rat_at(test_position)));
        
	if(attempts >= 100) {
		/* We tried 100 times to generate a position
		** but they were all occupied.
		*/
		return INVALID_POSITION;
	}
	super_food_exists = 1;
	super_food_pos = test_position;
	return test_position;	
}

PosnType get_super_food_pos(void) {
	return super_food_pos;
}

void remove_super_food(void) {
	super_food_exists = 0;
	reset_superfood_status();
}

uint8_t is_super_food_at(PosnType pos) {
	if(pos == super_food_pos && super_food_exists) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t get_super_food_existence(void) {
	return super_food_exists;
}

