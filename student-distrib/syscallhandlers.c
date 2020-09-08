#include "syscallhandlers.h"
#include "paging.h"
#include "x86_desc.h"
#include "scheduling.h"

int32_t pid_array[MAX_PROCESSES] = {PID_FREE, PID_FREE, PID_FREE, PID_FREE, PID_FREE, PID_FREE};

/*int32_t halt (uint8_t status)*/
/*interface: halt the process by switching back to the previous process's stack, first we extract the */
/*           the current pcb pointer and close all the files within the current pcb. Then we make sure */
/*           that halting in the base shell will only restart the current process. Then we free the pid*/
/*           for current process then link back to the parent program*/
/*input: status of the process */
/*output: nothing*/
/*return value: will never reaches to the part that returns the value*/
/*side effect: halt the current process and perform a contet switch back to the parent process*/
int32_t halt (uint8_t status){
    /* declare local variables */
    cli();
    pcb_t* parent_pcb;
    uint32_t parent_pid, parent_esp, parent_ebp;
    int i;
    clear_buffer();
    t_s[cur_ter].newline_flag = 0;
    /* extract pcb pointer from current running pid */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    
    /* close all fds */
    for (i = FIRST_NON_STD; i < NUM_FD; i++){
        close(i);
    }

    /* if exit on base shell, restart shell */
    if (!pcb_pointer->is_haltable){
        pid_array[pcb_pointer->pid] = PID_FREE;
        t_s[cur_ter].base_shell_pid = -1;
        execute((uint8_t*)"shell");
    }

    /* grab parent pcb and pid */
    parent_pcb = (pcb_t*)pcb_pointer->parent_pcb;
    parent_pid = parent_pcb->pid;

    /* reset current running pid to parent pid */
    t_s[cur_ter].current_running_pid = parent_pid;

    /* set values in tss */
    tss.esp0 = _8MB - parent_pid * _8KB - END_OFFSET;
    tss.ss0 = KERNEL_DS;

    /* load program page of parent */
    setup_program_page(parent_pid); 

    /* close vidmap page */
    if(cur_ter != disp_ter) {
        close_different_vidmap_page(cur_ter);
    } else {
        close_vidmap_page();
    }

    /* update global status */
    /* set the correct status to return from the parent process*/
    if (status ==  EXCEPTION_STATUS){
        t_s[cur_ter].global_status = EXCEPTION_CODE;
    } else {
        t_s[cur_ter].global_status = (int32_t) status;
    }

    /* free pid */
    pid_array[pcb_pointer->pid] = PID_FREE;

    /* reset fish running flag */
    t_s[cur_ter].fish_running = 0;

    /* link back to parent program */
    parent_esp = pcb_pointer->esp;
    parent_ebp = pcb_pointer->ebp;
    t_s[cur_ter].parent = (pcb_t*) pcb_pointer->parent_pcb;
    sti();
    /*reset esp and ebp to return to the parent process*/
    asm volatile("movl %0, %%esp            \n\
                  movl %1, %%ebp            \n\
                  jmp halt_return           \n\
                  "
                  :
                  :"r"(parent_esp), "r"(parent_ebp)
                  : "esp", "ebp"
                  );
    /* never reach here*/
    return 0;
}
/*int32_t execute (uint8_t status)*/
/*interface: First, read the command and find if it exists a corresponding file that's executable*/
/*           after set up the page for user, we copy the file content into the corresponding page, */
/*           next, we make an pcb object on the correct address on the kernel page, and at the end we*/
/*           push in the args for context switch.*/
/*input: command to be excuted*/
/*output: none*/
/*return value: -1 -- invalid input, or reach maximum #'s process*/
/*              256 -- when a program is terminated by exception*/ 
/*              anything in between -- defined by user program */
/*side effect: excute the command, context switch to the user stack*/
int32_t execute (const uint8_t* command){
    /* declare local variables */
    cli();
    dentry_t dentry;
    int32_t filesize, i, j;
    int8_t magic_number[4] = {0x7f, 0x45, 0x4c, 0x46}; // magic number to check for executable
    int8_t buf[NUM_METADATA_BITS];
    uint32_t user_ds;
    uint32_t user_esp;
    uint32_t user_cs;
    uint32_t entry_point;
    pcb_t* pcb;
    uint32_t v_addr = V_ADDR_START;
    uint32_t mem_start = PROGRAM_START;

    /*filter the command*/    
    strcpy((int8_t*) global_command, (int8_t*) command);            // copy the current command into the global variable, global_command, which will hold the arguments
    uint8_t* filtered_command = (uint8_t*) command;        

    j = 0;                                      // index to keep track for the global command to know where the argument starts                 

    /*check for invalid argument*/
    if(!command)
    {
        sti();
        return -1;
    }

    /*remove the spaces before the command argument */
    while(*filtered_command == ' ' || *filtered_command == '\0')
    {
        filtered_command++;
        j++;
    }

    /*filter so the filtered_command ends with only the command and none of the arguments*/
    i = 0;                                    
    while(1)
    {
        if(filtered_command[i] == '\0' || filtered_command[i] == ' ')
        {
            filtered_command[i] = '\0';
            break;
        }
        i++;
        j++;
    }
    
    /* replace the global_command with the actual argument */
    for(i = 0; global_command[j] != '\0'; j++) {
        global_command[i++] = global_command[j];
    }

    /* add end of string */
    global_command[i] = '\0';

    /* check for valid executable */
    /* check if file existed*/
    if (read_dentry_by_name(filtered_command, &dentry) == -1)
    {
        sti();
        return -1;
    }

    /* check if the file is of type file*/
    if (dentry.filetype != FILE_CODE)
    {
        sti();
        return -1;
    }

    /* filesize */
    filesize = inodes[dentry.inode_num].length;

    /* check if the file holds valid context*/
    if (read_data(dentry.inode_num, 0, (uint8_t*)buf, NUM_METADATA_BITS) == 0)
    {
        sti();
        return -1;
    }

    /* check if the file's magic number is excutable */
    if (strncmp(buf, magic_number, NUM_MAGIC_BITS) != 0)
    {
        sti();
        return -1;
    }

    /* check for open process */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        if (pid_array[i] == PID_FREE)
        {
            pid_array[i] = PID_TAKEN;
            break;
        }
    }

    /* check if we reach the maximum #'s of processes allows*/
    if (i == MAX_PROCESSES)
    {
        sti();
        return -1;
    }

    /* valid executable, begin executing */

    /* create pcb */
    pcb = (pcb_t*) (KERNEL_BOTTOM - (i + 1) * _8KB);

    /* check if current execution is base shell */
    if (t_s[cur_ter].base_shell_pid == -1)
    {
        pcb->is_haltable = 0;
        t_s[cur_ter].base_shell_pid = i;
    } else {
        pcb->is_haltable = 1;
    }

    /* set current running pid */
    t_s[cur_ter].current_running_pid = i;

    /* extract entry address from metadata bytes 24-27 */
    uint8_t entry_addr[4] = {buf[24], buf[25], buf[26], buf[27]};

    /* set up paging: maps virtual addr to new 4MB physical page, set up page directory entry */
    setup_program_page(i);

    /* copy entire executable into virtual memory starting at virtual addr 0x08048000 */
    read_data((uint32_t)dentry.inode_num, (uint32_t)0, (uint8_t*)mem_start, (uint32_t)filesize);


    /* fill in pcb */
    pcb->pid = i;
    pcb->term_number = cur_ter;

    /* update parent */
    pcb->parent_pcb = t_s[cur_ter].parent;
    t_s[cur_ter].parent = (pcb_t*) (KERNEL_BOTTOM - (i + 1) * _8KB);

    /* fill in stdin */
    pcb->fdarray[0].f_ops_pointer = &stdin_op_table;
    pcb->fdarray[0].f_ops_pointer->read = &terminal_read;
    pcb->fdarray[0].f_ops_pointer->write = &stdin_write;
    pcb->fdarray[0].f_ops_pointer->open = &terminal_open;
    pcb->fdarray[0].f_ops_pointer->close = &terminal_close;
    pcb->fdarray[0].file_pos = 0;
    pcb->fdarray[0].inode = 0;
    pcb->fdarray[0].flags = FILE_CLOSED;

    /* fill in stdout */
    pcb->fdarray[1].f_ops_pointer = &stdout_op_table;
    pcb->fdarray[1].f_ops_pointer->write = &terminal_write;
    pcb->fdarray[1].f_ops_pointer->read = &stdout_read;
    pcb->fdarray[1].f_ops_pointer->open = &terminal_open;
    pcb->fdarray[1].f_ops_pointer->close = &terminal_close;
    pcb->fdarray[1].file_pos = 0;
    pcb->fdarray[1].inode = 0;
    pcb->fdarray[1].flags = FILE_CLOSED;


    /* get current value of esp and ebp */
    asm volatile (" movl %%esp, %0      \n\
                    movl %%ebp, %1      \n\
                  "
                  : "=r"(pcb->esp), "=r"(pcb->ebp)
                  :
                  : "esp", "ebp"
    );

    /* prepare for context switch */
    tss.esp0 = _8MB - i * _8KB - END_OFFSET;
    tss.ss0 = KERNEL_DS;
    user_ds = USER_DS;
    user_esp = v_addr + _4MB - END_OFFSET;
    user_cs = USER_CS;
    entry_point = *((uint32_t*) entry_addr);

    /* jump to entry point (entry_point) */
    asm volatile (" push %0             \n\
                    push %1             \n\
                    pushfl              \n\
                    popl %%eax          \n\
                    orl $0x200, %%eax  \n\
                    pushl %%eax         \n\
                    push %2             \n\
                    push %3             \n\
                    iret                \n\
                    "
                    :
                    :"r"(user_ds), "r"(user_esp), "r"(user_cs), "r"(entry_point)
                    :"eax"
                    );
    /* halt return */
    asm volatile( "halt_return:        \n\
                   "
    );
    return t_s[cur_ter].global_status;
}

/*int32_t read (int32_t fd, void* buf, int32_t nbytes)*/
/*interface: First, check for validity of fd number, and sti to allow interrupt. then set the proper esp*/
/*           and direct to the read operation corresponding to the fd number*/
/*input: fd -- file descriptor number*/
/*       buf -- buffer to be written*/
/*       nbytes -- number of bytes to be written*/
/*output: none*/
/*return value: -1 -- failure*/
/*              whatever the directed read function returns -- success*/
/*side effect: direct to a read function*/
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    /* check for valid fd */
    if (fd < 0 || fd >= NUM_FD){
        return -1;
    }
    sti();

    /* extract pcb from current running pid */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* check if file is open or not */
    if (pcb_pointer->fdarray[fd].flags == FILE_OPEN){
        return -1;
    }
    return (*pcb_pointer->fdarray[fd].f_ops_pointer->read)(fd, buf, nbytes);
}

/*int32_t write (int32_t fd, const void* buf, int32_t nbytes)*/
/*interface: First, check for validity of fd number, and check if the file can be written then set the proper esp*/
/*           and direct to the write operation corresponding to the fd number*/
/*input: fd -- file descriptor number*/
/*       buf -- buffer to be written*/
/*       nbytes -- number of bytes to be written*/
/*output: written on the screen or change value in a file*/
/*return value: -1 -- failure*/
/*              whatever the directed read function returns -- success*/
/*side effect: direct to a write function*/
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    /* check for valid fd */
    if (fd < 0 || fd >= NUM_FD){
        return -1;
    }

    /* extract pcb from current running pid */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);
    
    /* check if file is open or not */
    if (pcb_pointer->fdarray[fd].flags == FILE_OPEN){
        return -1;
    }
    return (*pcb_pointer->fdarray[fd].f_ops_pointer->write)(fd, buf, nbytes);
}
/*int32_t open (const uint8_t* filename)*/
/*interface: First, check if a file can be opened, then find the file's matadata in the filesystem*/
/*           then fill in the pcb with the corresponding value*/
/*input: filename -- name of the file to be open*/
/*output: none*/
/*return value: -1 -- failure*/
/*              fd -- success*/
/*side effect: opened a file and fill in pcb with respect to their file type*/
int32_t open (const uint8_t* filename){
    int32_t i;
    int32_t open_flag = 0;
    /* extract pcb from current running pid */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* check for invalid argument*/
    if (filename == (uint8_t*)  ""){
        return -1;
    }
    /* check for open fds */
    for (i = 0; i < NUM_FD; i++){
        if (pcb_pointer->fdarray[i].flags == FILE_OPEN){
            open_flag = 1;
            break;
        }
    }
    /* check for if there are no open fds*/
    if (open_flag == 0 || i == 8){
        return -1;
    }
    /* local variables to use to fill in the fdarray */
    int32_t fd = i;
    dentry_t dentry;

    /* fill in fdarray based on file type */
    if (read_dentry_by_name(filename, &dentry) == 0){
        /* file */
        if (dentry.filetype == FILE_CODE){
            pcb_pointer->fdarray[fd].f_ops_pointer = &file_op_table;
            pcb_pointer->fdarray[fd].f_ops_pointer->read = &file_read;
            pcb_pointer->fdarray[fd].f_ops_pointer->write = &file_write;
            pcb_pointer->fdarray[fd].f_ops_pointer->open = &file_open;
            pcb_pointer->fdarray[fd].f_ops_pointer->close = &file_close;
            pcb_pointer->fdarray[fd].inode = dentry.inode_num;
            pcb_pointer->fdarray[fd].file_pos = 0;
            pcb_pointer->fdarray[fd].flags = FILE_CLOSED;
        /* directory */
        } else if (dentry.filetype == DIR_CODE){
            pcb_pointer->fdarray[fd].f_ops_pointer = &dir_op_table;
            pcb_pointer->fdarray[fd].f_ops_pointer->read = &dir_read;
            pcb_pointer->fdarray[fd].f_ops_pointer->write = &dir_write;
            pcb_pointer->fdarray[fd].f_ops_pointer->open = &dir_open;
            pcb_pointer->fdarray[fd].f_ops_pointer->close = &dir_close;
            pcb_pointer->fdarray[fd].inode = 0;
            pcb_pointer->fdarray[fd].file_pos = 0;
            pcb_pointer->fdarray[fd].flags = FILE_CLOSED;
        /* rtc */
        } else if (dentry.filetype == RTC_CODE){
            pcb_pointer->fdarray[fd].f_ops_pointer = &rtc_op_table;
            pcb_pointer->fdarray[fd].f_ops_pointer->read = &rtc_read;
            pcb_pointer->fdarray[fd].f_ops_pointer->write = &rtc_write;
            pcb_pointer->fdarray[fd].f_ops_pointer->open = &rtc_open;
            pcb_pointer->fdarray[fd].f_ops_pointer->close = &rtc_close;
            pcb_pointer->fdarray[fd].inode = 0;
            pcb_pointer->fdarray[fd].file_pos = 0;
            pcb_pointer->fdarray[fd].flags = FILE_CLOSED;
        }
        if(pcb_pointer->fdarray[fd].f_ops_pointer->open(filename) == -1) {
            return -1; 
        }
        return fd;
    }
    return -1;
}

/*close (int32_t fd)*/
/*interface: First, check if a file can be closed and its fd's validity*/
/*           then free the entry of the file in the fdarray in the corresponding process*/
/*input: fd -- fd number of the file*/
/*output: none*/
/*return value: -1 -- failure*/
/*              0 -- success*/
/*side effect: free a file from fd array*/

int32_t close (int32_t fd){
    /* extract pcb from current running pid */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* check for valid fd */
    if (fd >= NUM_FD || fd < FIRST_NON_STD){
        return -1;
    }

    /* check if file open or not */
    if (pcb_pointer->fdarray[fd].flags == FILE_OPEN){
        return -1;
    }

    /* call specific close function */
    if(pcb_pointer->fdarray[fd].f_ops_pointer->close(fd) == -1) {
        return -1;
    }

    /* reset fd */
    pcb_pointer->fdarray[fd].f_ops_pointer = 0;
    pcb_pointer->fdarray[fd].inode = 0;
    pcb_pointer->fdarray[fd].file_pos = 0;
    pcb_pointer->fdarray[fd].flags = FILE_OPEN;
    return 0;
}


/*  int32_t getargs (uint8_t* buf, int32_t nbytes) */
/*   interface: reads the program's command line arguments into a user-level buffer*/
/*   input: buf -- buffer to be written */
/*       nbytes -- number of bytes to be written */
/*   output: none */
/*   return value: -1 -- failure */
/*                  0 on success */
/*   side effect: */
int32_t getargs (uint8_t* buf, int32_t nbytes)
{
    /* check for invalid arguments */
    if(global_command == NULL || buf == NULL)
        return -1;    
    if(strlen((int8_t*) global_command) > ARG_MAX_LENGTH)
        return -1;
    
    /* filter any spaces at the beginning of the argument*/
    uint8_t* filtered_command = global_command;

    while(*filtered_command == ' ')
    {
        filtered_command++;
    }
    /*check for no argument*/
    if(filtered_command[0] == '\0') {
        return -1;
    }

    /* copy the filtered argument into the buffer */
    strncpy((int8_t *) buf, (int8_t *) filtered_command, nbytes);

    /* reset global command */
    int32_t i;
    for(i = 0; i < ARG_MAX_LENGTH; i++) {
        global_command[i] = 0;
    }

    /*return 0 on success*/
    return 0;

}

/*  int32_t vidmap (uint8_t** screen_start) */
/*   interface: maps the text-mode video memory into user space at a pre-set virtual address */
/*   input: screen_start - double pointer to the video mem in user space */
/*   output:  */
/*   return value: -1 -- failure */
/*                  0 on success */
/*   side effect: sets up a page with the video mem with user level privilege */
int32_t vidmap (uint8_t** screen_start){
    /* check for valid screen_start */
    if (screen_start == NULL || (screen_start >= (uint8_t **) KERNEL_ADDR && screen_start <= (uint8_t **) (KERNEL_BOTTOM))){
        return -1;
    }
    /* set up the vidmap page */
    setup_vidmap_page();
    
    /* set screenstart to virtual address */
    *screen_start = (uint8_t * ) VIDMAP_V_ADDR;

    /* mark fish as running */
    t_s[cur_ter].fish_running = 1;

    return 0;
}

/* extra credit */
int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}

/* extra credit */
int32_t sigreturn (void){
    return -1;
}


