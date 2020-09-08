/*
 *
 * keyboard.h - defines used in interactions with the keyboard interrupt 
 * 
 */


#ifndef KEYBOARD_H
#define KEYBOARD_H
/* defined constant */
#define KEYBOARD_DATA_REG 0x60
#define MAP_SIZE     128     //the size of keyboard map
#define KEYBOARD_IRQ 1       //the irq num for keybaord 
#define BUF_SIZE     128     //maximum number of chars 127 buffer can hold + line feed
#define NUM_TERMS    3
#define PAGE_SIZE    4096
#define ADDR_ALIGN      12

uint32_t cur_ter;
uint32_t disp_ter;
int32_t keyboard_flag;
int32_t currently_typed_term;

/* terminal struct */
typedef struct terminal{
    int8_t kb_buf[BUF_SIZE];        // keyboard buffer for each terminal
    int32_t term_started;           // flag to see if the terminal has been enabled
    int32_t screen_x;               // x value of cursor
    int32_t screen_y;               // y value of cursor
    int8_t* video_mem_buf;          // address of background buffer for each page
    int32_t b_idx;                  // buffer index 
    int32_t r_idx;                  // read index
    int32_t current_running_pid;    // id of current running process
    int32_t base_shell_pid;         // id of base shell
    void* parent;                   // parent pcb
    int32_t global_status;          // return val for execute
    int32_t newline_flag;           // checks if enter has been pressed
    int32_t esp;                    // stack pointer
    int32_t ebp;                    // base pointer
    int32_t freq;                   // frequency
    int32_t counter;                // counter for virtualizing rtc
    int32_t fish_running;           // flag to see if fish is running in terminal
}   terminal_t;

extern terminal_t t_s[NUM_TERMS];

/*function essential to keyboards*/
void clear_buffer();
void init_keyboard();

/*function essential to terminal*/
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
void init_terminal();
void different_terminal(int32_t terminal_number);
#endif
