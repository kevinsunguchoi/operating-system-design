/* 
 * idt_asm.h - Defines used in interactions with the idt interrupt handlers
 */


#include "keyboard.h"
#include "rtc.h"
#include "idt.h"
#include "lib.h"
#include "syscallhandlers.h"

#ifndef IDT_ASM_H
#define IDT_ASM_H

/*asmHandler functions for interrupts*/
void asmHandler2();
void asmHandler32();
void asmHandler33();
void asmHandler40();
void asmHandler128();

#endif 
