/*
 * paging.c - functions to interact with paging
 *
 */
#include "paging.h"
#include "syscallhandlers.h"
#define NOT_PRESENT 0x02
#define PRW         0x3
#define PRWU        0x7
#define TERM1       1
#define TERM2       2
#define TERM3       3
#define PAGE_OFF    0xFFFF8
#define PT_OFF      0xFFFFFFFE


uint8_t attribute;
//void init_paging
//interface: first set every entry of page directory to be not_present then set every entry of first page  
//           table to be not_present, then we manually set up the kernal page and the video memory page 
//           and load the page directory and enable paging.
//intput: none
//output: none
//side effects: initialize paging and set up page directory and table
void init_paging()
{
    int i;
    //initialize page directory
    for(i = 0; i < DIRECTORY_SIZE; i++)
    {
        page_directory[i] = NOT_PRESENT;
    }
    //initialize page table
    for (i = 0; i < TABLE_SIZE; i++)
    {
        first_page_table[i] = NOT_PRESENT;  
    }
    // add video memory
    //add the pages for video memory
    //shift 12 to shift VIDEO 3 left of the hex so it's aligned in the base address section
    //OR with 3 to enable P and R/W
    first_page_table[VIDEO/ALIGNED_SIZE] =  VIDEO | PRW;

    /* enable the three background pages for different terminals */
    first_page_table[VIDEO/ALIGNED_SIZE + TERM1] = (VIDEO + PAGE_SIZE * TERM1) | PRW;
    first_page_table[VIDEO/ALIGNED_SIZE + TERM2] = (VIDEO + PAGE_SIZE * TERM2) | PRW;
    first_page_table[VIDEO/ALIGNED_SIZE + TERM3] = (VIDEO + PAGE_SIZE * TERM3) | PRW;

    

    //add the page
    //OR with 3 to enable P and R/W
    page_directory[0] = ((unsigned int)first_page_table) | 3; //set the first page with video memory
    //OR with 0x193 to enable P, R/W, PS, PCD, G 
    page_directory[1] = (KERNEL_ADDR | 0x193);//set the second page for kernel memory 

    //enable paging
    loadPageDirectory((unsigned int*)page_directory);
    enablePaging();

}
//void setup_program_page(int pid)
//interface: set up the user level page at index 32 and flush tlb
//input: the index number of pcb
//output: NONE
//return value: NONE
//side effect: set up the user level page at index 32 and flush tlb
void setup_program_page(int pid)
{
    //OR with 0x197 to enable P, R/W, PS, PCD, U/S, G 
    //and the index for the user page is 32 in page directory 
    page_directory[32] = ((_8MB + pid * _4MB) | 0x197);
    // flush tlb 
    asm volatile ("movl %cr3, %eax  \n\
                   movl %eax, %cr3  \n\
                   ");
}


/*  void setup_vidmap_page()        */
/*   interface: create a page for vid map with user level privilege*/
/*   input: */
/*   output: page in virtual for vid map with user level privilege*/
/*   return value: */
/*   side effect:  */
void setup_vidmap_page(){

    /* add the pages for video memory */

    /* OR with 7 to enable P and R/W and have it at user level privilege */
    vidmap_page[0] = VIDEO | PRWU;
    /* OR with 7 to enable P and R/W and user level privilege */
    page_directory[VIDMAP_IDX] = ((unsigned int) vidmap_page) | PRWU;        //set the 16th PDE with video map

    /* flush tlb */
    asm volatile ("movl %cr3, %eax  \n\
                movl %eax, %cr3  \n\
                ");
}

/*  void close_vidmap_page() */
/*   interface: close vidmap page created with user level privilege */
/*   input: */
/*   output: */
/*   return value: */      
/*   side effect:  */
void close_vidmap_page(){
    /* close vidmap page */

    /* AND with 0xFFFF8 to mark the page as not present */
    vidmap_page[0] = VIDEO & PAGE_OFF;
    /* AND with 0xFFFFFFFE to mark the page to not present*/
    page_directory[VIDMAP_IDX] &= PT_OFF;

    /* flush tlb */
    asm volatile ("movl %cr3, %eax  \n\
                movl %eax, %cr3  \n\
                ");
}

/*  void close_vidmap_page() */
/*   interface: close vidmap page created with user level privilege */
/*   input: */
/*   output: */
/*   return value: */      
/*   side effect:  */
void remap_vidmap_page(int32_t current_ter)
{
    /* enable P R/W U for page and page table */
    vidmap_page[0] =  (VIDEO + PAGE_SIZE * (1 + current_ter)) | PRWU;
    page_directory[VIDMAP_IDX] = ((unsigned int) vidmap_page) | PRWU;        //set the 16th PDE with video map

    /* flush tlb */
    asm volatile ("movl %cr3, %eax  \n\
                   movl %eax, %cr3  \n\
                   ");
}

/*  void close_different_vidmap_page() */
/*   interface: close vidmap page if the current terminal isnt the displayed terminal */
/*   input: current_ter - terminal number that we want to close the vidmap page for*/
/*   output: */
/*   return value: */      
/*   side effect:  */
void close_different_vidmap_page(int32_t current_ter){

    /* AND with 0xFFFF8 to mark the page as not present */
    vidmap_page[0] = VIDEO & PAGE_OFF;

    /* mark pde as not present */
    page_directory[VIDMAP_IDX] &= PT_OFF;

    /* flush tlb */
    asm volatile ("movl %cr3, %eax  \n\
                movl %eax, %cr3  \n\
                ");

}

