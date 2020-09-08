/* 
 * paging.h - Defines used in interactions with the paging interrupt
 * controller
 */
#ifndef PAGING_H
#define PAGING_H

#ifndef ASM

#include "lib.h"
#include "paging_init.h"
#define DIRECTORY_SIZE 1024
#define TABLE_SIZE     1024
#define ALIGNED_SIZE   4096
#define KERNEL_ADDR 0x400000
#define VIDMAP_IDX      16


//allocate memories for page_directory and first page_table
uint32_t page_directory[DIRECTORY_SIZE] __attribute__((aligned(ALIGNED_SIZE)));
uint32_t first_page_table[TABLE_SIZE] __attribute__((aligned(ALIGNED_SIZE)));
uint32_t vidmap_page[TABLE_SIZE] __attribute__((aligned(ALIGNED_SIZE)));
//function that calls to initialize paging
extern void init_paging();
void setup_program_page(int pid);
void setup_vidmap_page();
void close_vidmap_page();
void remap_vidmap_page(int32_t current_ter);
void close_different_vidmap_page(int32_t current_ter);
#endif
#endif 

