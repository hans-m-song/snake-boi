/*
** food.c
**
** Written by Peter Sutton edited by Hans Song
**
*/

#include <stdlib.h>
#include <stdio.h>

#include "position.h"
#include "superfood.h"
#include "rat.h"
#include "food.h"
#include "snake.h"
#include "board.h"
#include "timer0.h"

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

PosnType rat_pos;

PosnType add_rat(void) {
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
				is_super_food_at(test_position)));
        
    if(attempts >= 100) {
        /* We tried 100 times to generate a position
        ** but they were all occupied.
        */
        return INVALID_POSITION;
    }
	rat_pos = test_position;
	return rat_pos;
}

PosnType get_rat_pos(void) {
	return rat_pos;
}

void set_rat_pos(PosnType pos) {
	rat_pos = pos;
}

PosnType next_rat_pos(void) {
	int8_t new_x_pos, new_y_pos, attempts;
	attempts = 0;
	PosnType newPos;
	do {
		new_x_pos = x_position(rat_pos);
		new_y_pos = y_position(rat_pos);
		int8_t dirn = random()%4;
		if(dirn == LEFT) {
			if(new_x_pos == 1) {
				new_x_pos++;
			} else {
				new_x_pos--;
			}
		} else if(dirn == RIGHT) {
			if(new_x_pos == BOARD_WIDTH - 1) {
				new_x_pos--;
				} else {
				new_x_pos++;
			}
		} else if(dirn == UP) {
			if(new_y_pos == BOARD_HEIGHT - 1) {
				new_y_pos--;
			} else {
				new_y_pos++;
			}
		} else if(dirn == DOWN) {
			if(new_y_pos == 1) {
				new_y_pos++;
				} else {
				new_y_pos--;
			}
		}
		newPos = position(new_x_pos, new_y_pos);
		attempts++;
	} while(attempts < 100 &&
		(is_snake_at(newPos) || is_food_at(newPos) ||
		is_super_food_at(newPos) || position_out_of_bounds(newPos)));
	if(attempts >= 100) {
        /* We tried 100 times to generate a position
        ** but they were all occupied.
        */
        return INVALID_POSITION;
    }
	rat_pos = newPos;
	return rat_pos;
}

int8_t position_out_of_bounds(PosnType pos) {
	if (x_position(pos) >= BOARD_WIDTH - 1 &&
		x_position(pos) <= 2 &&
		y_position(pos) >= BOARD_HEIGHT - 1 &&
		y_position(pos) <= 2) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t is_rat_at(PosnType pos) {
	if(rat_pos == pos) {
		return 1;
	} else {
		return 0;
	}
}