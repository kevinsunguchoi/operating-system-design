#include "lib.h"
#include "filesystem.h"
#include "rtc.h"
#include "idt.h"
#include "tests.h"
#include "paging.h"
#include "x86_desc.h"

#ifndef SYSCALLHANDLERS_H
#define SYSCALLHANDLERS_H

#define MAX_PROCESSES       6
#define PCB_MASK            0xffffe000
#define FIRST_NON_STD       2
#define _8MB                0x800000
#define _4MB                0x400000
#define _8KB                0x2000
#define END_OFFSET          4
#define NUM_METADATA_BITS   40
#define V_ADDR_START        0x08000000
#define PROGRAM_START       0x08048000
#define NUM_MAGIC_BITS      4
#define PID_TAKEN           1
#define PID_FREE            0
#define KERNEL_BOTTOM       2 * KERNEL_ADDR
#define EXCEPTION_CODE      256
#define EXCEPTION_STATUS    255
#define VIDMAP_V_ADDR       0x04000000
#define ARG_MAX_LENGTH      128


/* global variable for storing parent pcb */
extern void* parent;

/* array for open processes, initalize all processes to free */
extern int32_t pid_array[MAX_PROCESSES];

/* holds status for execute */
int32_t global_status;

/* global command*/
uint8_t global_command[ARG_MAX_LENGTH];


/* system call handlers, interaces are in c file */
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

#endif 

