/* 
 * 
 * rtc.c - functions to interact with the rtc 
 * 
 */

#include "lib.h"
#include "rtc.h"
#include "i8259.h"
#include "keyboard.h"
#include "filesystem.h"

#define RTC_IRQ     8
#define BIT6        0x40
#define HIGH_MASK   0xF0

static volatile int8_t INT_RECEIVED = 0; //check if an rtc int is received
// static volatile int8_t rate;
/* rtc_init
 * DESCRIPTION: initializes the RTC, enables interrupt request for rtc
 * INPUT: nothing
 * OUTPUT: nothing
 * SIDE EFFECT: 
 */
void rtc_init() 
{
    disable_irq(RTC_IRQ);
    send_eoi(RTC_IRQ);
    outb(RTC_B, RTC_PORT);		 //select reg B and disabke nmi
    char prev = inb(RTC_DATA);	 //read the current value of register B
    outb(RTC_B, RTC_PORT);		 //reset
    outb(prev | BIT6, RTC_DATA); //or with 0x40 to enbable bit 6 of Reg B PIE

    /* set frequency to 1024 Hz */
    uint8_t rt = K_MAX_INT_RATE;
    outb(RTC_A, RTC_PORT);		// set index to register A, disable NMI
    int8_t curr = inb(RTC_DATA);	// get initial value of register A
    outb(RTC_A, RTC_PORT);		// reset index to A
    outb((curr & HIGH_MASK) | rt, RTC_DATA); //write only our rate to A. Note, rate is the bottom 4 bits.
    enable_irq(RTC_IRQ);         //enable the irq for rtc on pic 
}

/* handler40
 * DESCRIPTION: handler for RTC interrupts
 * INPUT: nothing
 * OUTPUT: nothing
 * SIDE EFFECT: 
 */
void handler40(){     
    send_eoi(RTC_IRQ);  
    pcb_t* pcb_pointer;
    int i;

    /* loop through terminals */
    for(i = 0; i < NUM_TERMS; i++){
        if(t_s[i].term_started){
            /* get pcb pointer */
            pcb_pointer = (pcb_t*)((_8MB - t_s[i].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);
            if(pcb_pointer->counter > 0) {
                /* decrement process counter each time an interrupt is received */
                pcb_pointer->counter--;
            }
        } 
    }
    outb(RTC_C, RTC_PORT);	// set index to register A, disable NMI
    inb(RTC_DATA);		    // retrieve rtc_data
}

//uint32_t rtc_open(const uint8_t* filename)
//interface: set the frequency to be 2HZ
//
//
//
int32_t rtc_open(const uint8_t* filename)
{     
    /* get pcb pointer */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* initialize rtc variables for pcb */
    pcb_pointer->rtc_freq = 2; // initial frequency is 2 Hz
    pcb_pointer->counter = 0;
    return 0;
}
//uint32_t rtc_close(int32_t fd)
//interface: nothing
//input: fd
//output: none
//return value: 0
//side effect: none
int32_t rtc_close(int32_t fd)
{
    //do nothing
    return 0;
}

//uint32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
//interface: block until the next interrupt, return 0
//input: fd:
//       buf:
//       nbytes:
//output: none
//return value: 0
//side effect: block until the next interrupt is received
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes)
{
    /* get pcb pointer */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* set counter equal to max frequency divided by process frequency */
    pcb_pointer->counter = K_MAX_INT_FREQ / pcb_pointer->rtc_freq;

    while(pcb_pointer->counter > 0)
    {
        // do nothing until counter hits 0
    }
    return 0;
}
//uint32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
//interface: write to rtc with rates corresponding to the correct frequency input
//input: buf: pointer to frequency
//output: none
//return value: -1 : failure
//              0  : succuss
//side effect: change the interrupt rate in rtc
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
    uint16_t freq = *((int*)buf);
    //check if buf is NULL, frequency is NULL, frequency is power of 2, frequency is within the range
    if(!buf || freq <=0 || (freq & (freq-1)) || freq > K_MAX_INT_FREQ)
        return -1;    

    /* get pcb pointer */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* set rtc freq for process */
    pcb_pointer->rtc_freq = (int) freq; 
    return 0;
}

