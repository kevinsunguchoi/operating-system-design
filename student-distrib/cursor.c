#include "cursor.h"

#define PORT        0x3d4
#define DATA        0x3d5
#define MASK        0xff
#define BIT_SHIFT   0x08
#define HIGH_CURSOR 0x0e
#define LOW_CURSOR  0x0f


/* void update_cursor(pos);
 * Inputs: pos: position on screen for cursor
 * Return Value: none
 * Function: Clears video memory */
void update_cursor(uint16_t pos){
    outb(LOW_CURSOR, PORT);

    /* column */
    outb((uint8_t)(pos & MASK), DATA);
    outb(HIGH_CURSOR, PORT);

    /* row */
    outb((uint8_t)((pos >> BIT_SHIFT) & MASK), DATA);
}

