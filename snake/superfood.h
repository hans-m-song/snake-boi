/*
** food.h
**
** Written by Peter Sutton
**
** Function prototypes for those functions available externally
*/

/* Guard band to ensure this definition is only included once */
#ifndef SUPERFOOD_H_
#define SUPERFOOD_H_

#include <inttypes.h>
#include "position.h"

PosnType add_super_food(void);

PosnType get_super_food_pos(void);

void remove_super_food(void);

uint8_t is_super_food_at(PosnType pos);

uint8_t get_super_food_existence(void);

#endif
