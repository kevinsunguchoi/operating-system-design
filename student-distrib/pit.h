#ifndef PIT_H
#define PIT_H

#include "i8259.h"
#include "scheduling.h"

#define CH0_DATA    0x40
#define CMD_REG     0x43
#define HIGH        8
#define CMD_BYTE    0x36
#define INPUT_CLK   1193180
#define PIT_IRQ     0

void handler32();
void init_pit(int hz);


#endif






