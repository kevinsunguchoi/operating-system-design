/*
 * keyboard.c - functions to interact with the keyboard
 *
 */

#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "syscallhandlers.h"
#include "scheduling.h"
static volatile uint8_t CAPS_PRESSED = 0;
static volatile uint8_t SHIFT_PRESSED = 0;
static volatile uint8_t CONTROL_PRESSED = 0;
static volatile uint8_t ALT_PRESSED = 0;
static volatile uint8_t TAB_PRESSED = 0;

terminal_t t_s[NUM_TERMS];
/*
 * keyboard_map is a scancode table used to layout a standard US keyboard
 */
uint8_t keyboard_map[MAP_SIZE] =
{
  0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  
  ' ','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,            
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 
  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',	
};
/*
  shift_map is a scancode table used to layout a shifted standard US keyboard
*/
uint8_t shift_map[MAP_SIZE] =
{
  0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  
  ' ','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,            
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
};
/*
 * void handler33()
 * 
 * Description: Handler for the keyboard. Checks if the keycode
 * retrieved from the scancode is positive and its corresponding
 * scancode doesn't return 0 and if both conditions are met,
 * will putc the corresponding key onto the screen
 * 
 * No inputs nor outputs
 */
void handler33() 
{
    cli();

    disable_irq(KEYBOARD_IRQ);
    send_eoi(KEYBOARD_IRQ);
    int8_t key = inb(KEYBOARD_DATA_REG); //retrieves keycode
    /* checking for any functionality keys being released or pressed*/

    /*when left shift and right shift is released*/
    if((key == -86 || key == -74) && SHIFT_PRESSED == 1) //scancodes of releasing left shift and right shift are -86, -74
      SHIFT_PRESSED = 0; //set shift pressed to 0
    
    /*when alt is released*/
    if((key == -72) && ALT_PRESSED == 1) //scancode of releasing alt is -72
      ALT_PRESSED = 0; //set alt pressed to 0
    
    /*when control is released*/
    if((key == -99) && CONTROL_PRESSED == 1) //scancode of releasing control is -99
      CONTROL_PRESSED = 0; //set control pressed to 0

    /*check if enter is presssed, scancode for enter is 28*/
    if(key == 28)
    {
      t_s[disp_ter].newline_flag = 1; //set the newline flag to be 1
      t_s[disp_ter].kb_buf[t_s[disp_ter].b_idx] = '\n'; //append newline at the end
      keyboard_flag = 1;
      putc('\n');
      enable_irq(KEYBOARD_IRQ);
      return;
    }
    
    /*when tab is pressed*/
    if (key == 15) 
    {
      TAB_PRESSED = 1;
    }
    else 
    {
      TAB_PRESSED = 0;
    }

    /*when alt is pressed*/
    if (key == 56)
      ALT_PRESSED = 1;

    /*when control is pressed*/
    if (key == 29)
      CONTROL_PRESSED = 1;

    /*when left shift and right is pressed*/
    if(key == 42 || key == 54) //scancodes of pressing left shift and right shift are 42, 54
      SHIFT_PRESSED = 1;

    /*check if backspace is pressed*/
    if (key == 14)
    {
      if(t_s[disp_ter].b_idx) //check if the index is 0
      {
        t_s[disp_ter].kb_buf[t_s[disp_ter].b_idx] = 0;
        t_s[disp_ter].b_idx--;
        keyboard_flag = 1;
        backspace();
      }
      enable_irq(KEYBOARD_IRQ);
      return;
    }

    /*check for terminal switch*/
    if (ALT_PRESSED == 1)
    {
      switch (key)
      {
      case 0x3b: // terminal 1
        different_terminal(0);
        enable_irq(KEYBOARD_IRQ);
        return;
      case 0x3c: // terminal 2
        different_terminal(1);
        enable_irq(KEYBOARD_IRQ);
        return;
      case 0x3d: // terminal 3
        different_terminal(2);
        enable_irq(KEYBOARD_IRQ);
        return;
      }
    }

    /* printing onto screen */
    if (key > 0) 
    { 
      int ascii_val = keyboard_map[(int)key]; //ascii value of the key we just obtained
      int sh_ascii_val = shift_map[(int)key]; //shift ascii value of the key we just obtained
      //checking if ctrl + l or ctrl + L
      if (CONTROL_PRESSED == 1 && (ascii_val == 'l' || sh_ascii_val == 'L')) 
      {
        t_s[disp_ter].b_idx = 0;
        clear_buffer(); //clear th buffer 
        clear();        //clear the screen
        enable_irq(KEYBOARD_IRQ);
        return;
      }
      /* when capslock is pressed */
      if(key == 58)
      {
        CAPS_PRESSED ^= 1; //caps lock is enabled
      }

      //if the key thats a character
      if(ascii_val >= 97 && ascii_val <= 122 && t_s[disp_ter].b_idx < 127) //the range of lower case ascii letters are from 97 to 122
      {
        //if only one of those is pressed 
        if(CAPS_PRESSED ^ SHIFT_PRESSED) 
        {
          keyboard_flag = 1;
          putc(sh_ascii_val); //print the caps char of the key, and when a shift is not pressed 
          t_s[disp_ter].kb_buf[t_s[disp_ter].b_idx] = sh_ascii_val;
          t_s[disp_ter].b_idx++;
        }
        else //when both are pressed or neither is pressed
        {
            keyboard_flag = 1;
          putc(ascii_val);
          t_s[disp_ter].kb_buf[t_s[disp_ter].b_idx] = ascii_val;
          t_s[disp_ter].b_idx++;
        }
      }
      else if(ascii_val != 0 && t_s[disp_ter].b_idx < 127) //if the input is not a character 
      {
        if(SHIFT_PRESSED)
        {
            keyboard_flag = 1;
          putc(sh_ascii_val);
          t_s[disp_ter].kb_buf[t_s[disp_ter].b_idx] = sh_ascii_val;
          t_s[disp_ter].b_idx++;
        }
        else
        {
            keyboard_flag = 1;
          putc(ascii_val);
          t_s[disp_ter].kb_buf[t_s[disp_ter].b_idx] = ascii_val;
          t_s[disp_ter].b_idx++;
        }
      }
    }
    enable_irq(KEYBOARD_IRQ); //end of interrupt
    sti();
}

/*
 * void init_keyboard()
 * 
 * Description: enables interrupt request into initialize keyboard
 * 
 * No inputs nor outputs
 */
void init_keyboard()
{
    disable_irq(KEYBOARD_IRQ);
    send_eoi(KEYBOARD_IRQ);
    enable_irq(KEYBOARD_IRQ);
}
/*
 * void clear_buffer()
 * interface: clear the key_board buffer and reset the buffer index and read index
 * input: none
 * output: none
 * return value: 0
 * side effects: clear the buff index
 */
void clear_buffer()
{
  uint32_t i = 0;
  for (i = 0; i < BUF_SIZE; i++)
  {
    t_s[cur_ter].kb_buf[i] = 0; //clearing the buffer
  }
  /* reseting the index */
  t_s[cur_ter].b_idx = 0;
}
/*
 * void terminal_open(const uint8_t* filename)
 * interface: clear the keyboad buffer
 * input: filename
 * output: one
 * return value: 0
 * side effects: nothing
 */
int32_t terminal_open(const uint8_t* filename)
{
  clear_buffer();
  return 0;  //do nothing
}
/*
 * void terminal_open(int32_t fd)
 * interface: do nothing
 * input: fd
 * output: none
 * return value: 0
 * side effects: nothing
 */
int32_t terminal_close(int32_t fd)
{
    return 0; //do nothing
}
/*
 * void terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * interface: wait until enter is pressed then read the the number of bytes from key board buffer into the buf
 * input: fd : not used
 *        buf: user-level buffer to copy to
 *        nbytes: number of bytes to copy to
 * output: none
 * return value: success - number of bytes read
 *               fail    - -1
 * side effects: read the the number of bytes into the buf
 */
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes)
{
    sti();
    if(nbytes < 1 || buf == NULL)
        return -1;
    //wait until the newline signal is triggered
    while(!t_s[cur_ter].newline_flag || cur_ter != disp_ter);
    uint32_t i; //loop counter
    uint32_t count = 0; //number of bytes read
    uint8_t *buffer = (uint8_t *)buf; //cast the buffer
    for(i = 0; i < nbytes && i <= t_s[cur_ter].b_idx; i++)
    {
      buffer[i] = t_s[cur_ter].kb_buf[i]; //copy key board buffer to buf
      count++; //increment the count every time a byte is read into buf
      /*when we reach the end*/
    }
    clear_buffer();
      t_s[cur_ter].newline_flag = 0;
    // //increment the number of times a keyboard string has being read
    return count;
}
/*
 * void terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * interface: write the the number of bytes from input buffer on screen
 * input: fd : not used
 *        buf: user-level buffer to write
 *        nbytes: number of bytes to write
 * output: output buf on the screen
 * return value: success - number of bytes write
 *               fail    - -1
 * side effects: write the number of bytes from the buffer on screen
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    cli();
    /*check if buf is NULL*/
    if(buf == NULL)
        return -1;
    uint32_t i; //loop counter
    uint8_t * a = (uint8_t*) buf; //cast the void ptr
    for(i = 0; i < nbytes; i++)
    {
        putc(a[i]); //output char on screen
    }
    sti();
    return i;
}

/* init_terminal
 * DESCRIPTION: initialize all terminal struct members and start up first shell
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECT: 
 */
void init_terminal(){
    int32_t i, j;
    for (i = 0; i < NUM_TERMS; i++){
        for (j = 0; j < BUF_SIZE; j++)
        {
            t_s[i].kb_buf[j] = 0; //clearing the buffer
        }
        /* initialize all members in struct */
        t_s[i].b_idx = 0;
        t_s[i].r_idx = 0;
        t_s[i].screen_x = 0;
        t_s[i].screen_y = 0;
        t_s[i].term_started = 0;
        t_s[i].video_mem_buf = (int8_t*)(VIDEO + PAGE_SIZE * (i + 1));
        t_s[i].base_shell_pid = -1;
        t_s[i].parent = NULL;
        t_s[i].fish_running = 0;
    }
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++)
    {
        *(uint8_t *)(VIDEO + (i << 1)) = ' ';
        *(uint8_t *)(VIDEO + (i << 1) + 1) = color[disp_ter];
    }
    /* initialize cur_ter and disp_ter */
    cur_ter = 0;
    disp_ter = 0;
    screen_x = 0;
    screen_y = 0;
    update_cursor(0);
    /* start the first shell */
    t_s[0].term_started = 1;
    execute((uint8_t*)"shell");
    
}

/* different_terminal
 * DESCRIPTION: switch the terminal that the user is currently viewing
 * INPUT: terminal_number - terminal that the user is switching to
 * OUTPUT: none
 * SIDE EFFECT: 
 */
void different_terminal(int32_t terminal_number){
    uint16_t pos;
    
    enable_irq(KEYBOARD_IRQ);

    /* copy physical memory to background buffer of old terminal and store the current screen_x and screen_y */
    memcpy((uint8_t*)((VIDEO/ALIGNED_SIZE + 1 + disp_ter) << ADDR_ALIGN), (uint8_t*)(VIDEO/ALIGNED_SIZE << ADDR_ALIGN), PAGE_SIZE);
    t_s[disp_ter].screen_x = screen_x;
    t_s[disp_ter].screen_y = screen_y;

    /* change display terminal */
    disp_ter = terminal_number;

    /* copy video memory of new terminal buffer into actual video memory, restore screen positions and update the cursor */
    memcpy((uint8_t*)(VIDEO/ALIGNED_SIZE << ADDR_ALIGN), (uint8_t*)((VIDEO/ALIGNED_SIZE + 1 + disp_ter) << ADDR_ALIGN), PAGE_SIZE);
    screen_x = t_s[disp_ter].screen_x;
    screen_y = t_s[disp_ter].screen_y;
    pos = screen_y * NUM_COLS + screen_x;
    update_cursor(pos);

}
