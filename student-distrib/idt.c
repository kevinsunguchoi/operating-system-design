/*
 * idt.c - functions to interact with idt
 *
 */

#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "idt_asm.h"

/* void handler0()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE 
    OUTPUT: NONE
*/

void handler0(){
    printf("Divide by zero");
    halt(EXCEPTION_STATUS);
    while(1){

    }
}

/* void handler1()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler1(){
    printf("Debug Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler2()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler2(){
    printf("NMI Interrupt");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler3()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler3(){
    printf("Breakpoint Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler4()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler4(){
    printf("Overflow Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler5()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler5(){
    printf("BOUND Range Exceeded Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler6()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler6(){
    printf("Invalid Opcode Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler7()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler7(){
    printf("Device Not Available Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler8()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler8(){
    printf("Double Fault Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler9()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler9(){
    printf("Debug Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler10()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler10(){
    printf("Invalid TSS Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler11()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler11(){
    printf("Segment Not Present");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler12()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler12(){
    printf("Stack Fault Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler13()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler13(){
    printf("General Protection Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler14()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler14(){
    printf("Page Fault Exception\n");
    uint32_t value, error_code;
    asm volatile(
        "movl %%cr2, %0     \n\
         movl 8(%%esp), %1"
        :"=r"(value), "=r"(error_code)
        :
        :"memory"
    );
    /* Print the information of the page fault*/
    printf("linear address: %x\n", value);
    printf("error code: %x\n", error_code);
    register int32_t esp asm ("esp");
    printf("process number: %x\n", esp & 0xFFFFE000);
    halt(EXCEPTION_STATUS);
}

/* void handler15()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler15(){
    printf("Nothing");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler16()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler16(){
    printf("x87 FPU Floating Point Error");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler17()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler17(){
    printf("Alignment Check Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler18()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler18(){
    printf("Machine Check Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler19()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler19(){
    printf("SIMD Floating Point Exception");
    halt(EXCEPTION_STATUS);
    while(1){
        
    }
}

/* void handler128()
    DESCRIPTION: prints the descriptions of the specific interrupt called
    and freezes the kernel for an indefinite amount of time
    INPUT: NONE
    OUTPUT: NONE
*/
void handler128(){
    printf("System Call");
    while(1){
        
    }

}

/* init_idt()
 * DESCRIPTION: initializes the idt
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECT:
 */
void init_idt() {
    idt_desc_t desc;

    /* Setting specific bits for interrupt gate */
    desc.present = 0x1;
    desc.dpl = 0x0;
    desc.reserved0 = 0x0;
    desc.size = 0x1;
    desc.reserved1 = 0x1;
    desc.reserved2 = 0x1;
    desc.reserved3 = 0x0;
    desc.reserved4 = 0x0;
    desc.seg_selector = KERNEL_CS;

    /* Sets IDT entries in the interrupt descriptor table */
    SET_IDT_ENTRY(desc, handler0);
    idt[0] = desc; //set desc equal to this index (0) of IDT

    SET_IDT_ENTRY(desc, handler1);
    idt[1] = desc; //set desc equal to this index (1) of IDT

    SET_IDT_ENTRY(desc, handler2);
    idt[2] = desc; //set desc equal to this index (2) of IDT

    SET_IDT_ENTRY(desc, handler3);
    idt[3] = desc; //set desc equal to this index (3) of IDT

    SET_IDT_ENTRY(desc, handler4);
    idt[4] = desc; //set desc equal to this index (4) of IDT

    SET_IDT_ENTRY(desc, handler5);
    idt[5] = desc; //set desc equal to this index (5) of IDT

    SET_IDT_ENTRY(desc, handler6);
    idt[6] = desc; //set desc equal to this index (6) of IDT

    SET_IDT_ENTRY(desc, handler7);
    idt[7] = desc; //set desc equal to this index (7) of IDT

    SET_IDT_ENTRY(desc, handler8);
    idt[8] = desc; //set desc equal to this index (8) of IDT

    SET_IDT_ENTRY(desc, handler9);
    idt[9] = desc; //set desc equal to this index (9) of IDT

    SET_IDT_ENTRY(desc, handler10);
    idt[10] = desc; //set desc equal to this index (10) of IDT

    SET_IDT_ENTRY(desc, handler11);
    idt[11] = desc; //set desc equal to this index (11) of IDT

    SET_IDT_ENTRY(desc, handler12);
    idt[12] = desc; //set desc equal to this index (12) of IDT

    SET_IDT_ENTRY(desc, handler13);
    idt[13] = desc; //set desc equal to this index (13) of IDT

    SET_IDT_ENTRY(desc, handler14);
    idt[14] = desc; //set desc equal to this index (14) of IDT

    SET_IDT_ENTRY(desc, handler16);
    idt[16] = desc; //set desc equal to this index (16) of IDT

    SET_IDT_ENTRY(desc, handler17);
    idt[17] = desc; //set desc equal to this index (17) of IDT

    SET_IDT_ENTRY(desc, handler18);
    idt[18] = desc; //set desc equal to this index (18) of IDT

    SET_IDT_ENTRY(desc, handler19);
    idt[19] = desc; //set desc equal to this index (19) of IDT
    
    /* Interrupts */
    SET_IDT_ENTRY(desc, asmHandler2);
    idt[2] = desc;                      // set desc equal to this index (2) of IDT

    SET_IDT_ENTRY(desc, asmHandler32);
    idt[32] = desc;

    SET_IDT_ENTRY(desc, asmHandler33);
    idt[33] = desc;                     //set desc equal to this index (33) of IDT

    SET_IDT_ENTRY(desc, asmHandler40);
    idt[40] = desc;                     //set desc equal to this index (40) of IDT

    /* System Call */
    desc.dpl = USER_DPL;
    SET_IDT_ENTRY(desc, asmHandler128);
    idt[SYSCALL_INT_NUM] = desc;

    /* Load IDT */
    lidt(idt_desc_ptr);
}


