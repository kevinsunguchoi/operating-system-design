#include "pit.h"

/* void handler32
    DESCRIPTION: called when PIT interrupts, calls scheduler
    INPUT: NONE 
    OUTPUT: NONE
*/
void handler32(){
    send_eoi(PIT_IRQ);
    schedule();
}

/* void init_pit
    DESCRIPTION: initializes PIT to hz frequency
    INPUT: hz - frequency to set pit to 
    OUTPUT: NONE
*/
void init_pit(int hz) {
    disable_irq(PIT_IRQ);
    send_eoi(PIT_IRQ);
    int divisor = INPUT_CLK / hz;       /* Calculate our divisor */
    outb(CMD_BYTE, CMD_REG);             /* Set our command byte 0x36 */
    outb(divisor & MASK, CH0_DATA);   /* Set low byte of divisor */
    outb(divisor >> HIGH, CH0_DATA);     /* Set high byte of divisor */
    enable_irq(PIT_IRQ);
}

