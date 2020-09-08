#include "scheduling.h"
#include "keyboard.h"


/* schedule
 * DESCRIPTION: called by PIT handler, switch the terminal that is currently processing
 * INPUT: none
 * OUTPUT: none
 * SIDE EFFECT: if terminal is not already started, just start a new shell
 */
void schedule(){
    cli();

    /* save esp and ebp of previous process */
    asm volatile("movl %%esp, %0            \n\
                movl %%ebp, %1            \n\
                "
                :"=r"(t_s[cur_ter].esp), "=r"(t_s[cur_ter].ebp)
                :
                :"memory"
                );

    /* change the currently executing terminal */
    cur_ter = (cur_ter + 1) % NUM_TERMS;

    /* terminal is already started */
    if (t_s[cur_ter].term_started == 1){
        /* remap virtual program page to physical program page */
        setup_program_page(t_s[cur_ter].current_running_pid);

        /* remap vidmap if fish is running in that terminal*/
        if (t_s[cur_ter].fish_running){
            if(cur_ter != disp_ter) {
                remap_vidmap_page(cur_ter);
            } else {
                setup_vidmap_page();
            }
        }

        /* set tss values */
        tss.esp0 = _8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET;
        tss.ss0 = KERNEL_DS;

        /* restore ebp and esp of new process */
        asm volatile("movl %0, %%esp           \n\
                  movl %1, %%ebp            \n\
                  "
                  :
                  :"r"(t_s[cur_ter].esp), "r"(t_s[cur_ter].ebp)
                  :"memory"
                  );
    /* else just start up a new shell */
    } else {
        t_s[cur_ter].term_started = 1;
        sti();
        execute((uint8_t*)"shell");
    }
    sti();
}




