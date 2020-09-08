/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
/* Initialize the 8259 PIC */
/* init_idt()
 * DESCRIPTION: initializes the 8259 PIC
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECT:
 */
void i8259_init(void) {

    /* Initial PIC Masks */
    master_mask = MASK;
    slave_mask = MASK;

    /* Set Masks */
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    /* Set master control words */
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);

    /* Set slave control words */
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    /* Enable slave PIC */
    enable_irq(SLAVE_PIN);
}

/* Disable (unmask) the specified IRQ */
/* disable_irq()
 * DESCRIPTION: Disable (unmask) the specified IRQ
 * INPUT: IRQ number
 * OUTPUT: none
 * SIDE EFFECT:
 */
void disable_irq(uint32_t irq_num) {
 
    /* Check if the irq comes from the slave or pic, then set mask accordingly */
    if(irq_num < SLAVE_OFFSET) {
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    } else {
        irq_num -= SLAVE_OFFSET;
        slave_mask = slave_mask | (1 << irq_num);
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* Enable (mask) the specified IRQ */
/* enable_irq()
 * DESCRIPTION: Enable (unmask) the specified IRQ
 * INPUT: IRQ number
 * OUTPUT: none
 * SIDE EFFECT:
 */
void enable_irq(uint32_t irq_num) {

    /* Check if the irq comes from the slave or pic, then set mask accordingly */
    if(irq_num < SLAVE_OFFSET) {
        master_mask = master_mask & ~(1 << irq_num);
        outb(master_mask, MASTER_8259_DATA);
    } else {
        irq_num -= SLAVE_OFFSET;
        slave_mask = slave_mask & ~(1 << irq_num);
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
/* send_eoi()
 * DESCRIPTION: Sends eoi for specified irq num
 * INPUT: IRQ number
 * OUTPUT: none
 * SIDE EFFECT:
 */
void send_eoi(uint32_t irq_num) {
    unsigned char eoi;
    /* If slave, send eoi to both slave and master */
    if (irq_num >= SLAVE_OFFSET){
        irq_num -= SLAVE_OFFSET;
        eoi = EOI | irq_num;
        outb(eoi, SLAVE_8259_PORT);
        eoi = EOI | SLAVE_PIN;
        outb(eoi, MASTER_8259_PORT);
    } else { // just send eoi to master
        eoi = EOI | irq_num;
        outb(eoi, MASTER_8259_PORT);
    }
}

