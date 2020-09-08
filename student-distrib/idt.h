/* 
 * 
 * idt.h - Defines used in interactions with the idt interrupt
 * 
 */

#ifndef IDT_H
#define IDT_H

#define KERNEL_CS   0x0010   
#define USER_DPL 3         //dpl for user
#define SYSCALL_INT_NUM 0x80 //system call irq numbers

/*
 * This function initializes IDT
 */ 
void init_idt();

/*
 * Handler functions
 */
void handler0();
void handler1();
void handler2();
void handler3();
void handler4();
void handler5();
void handler6();
void handler7();
void handler8();
void handler9();
void handler10();
void handler11();
void handler12();
void handler13();
void handler14();
void handler15();
void handler16();
void handler17();
void handler18();
void handler19();
void handler33();
void handler40();
void handler128();

#endif
