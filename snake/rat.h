/*
** food.h
**
** Written by Peter Sutton
**
** Function prototypes for those functions available externally
*/

/* Guard band to ensure this definition is only included once */
#ifndef RAT_H_
#define RAT_H_

#include <inttypes.h>
#include "position.h"

PosnType add_rat(void);

PosnType get_rat_pos(void);

void set_rat_pos(PosnType pos);

PosnType next_rat_pos(void);

uint8_t is_rat_at(PosnType pos);

int8_t position_out_of_bounds(PosnType pos);

#endif