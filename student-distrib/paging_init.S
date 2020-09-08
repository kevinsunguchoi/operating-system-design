/*
 *  idt_asm.S - assembly code for the paging functions
 *
 */
.text
.globl loadPageDirectory
.globl enablePaging

# void loadPageDirectory()
# interface: set the cr3 and cr4 to load the pagedirectory 
# input: pointer of the page directory table
# output: NONE
# side_effects: load the page directory
# clobbers: ebx, eax, 
loadPageDirectory:
    # stack set up
    pushl %ebp
    movl %esp, %ebp
    # callee save
    pushl %esi
    pushl %edi
    pushl %ebx
    pushl %eax
    # put the arg in eax 
    movl 8(%ebp), %eax
    # put cr3 in ebx and and it with 0000fff to clear the base address and preserve other bits
    movl %cr3, %ebx
    andl $0x0000fff, %ebx
    # or to obtain the address from the arg and move it to cr3
    orl %eax, %ebx
    movl %ebx, %cr3
    # store cr4 in eax, and and it with 7cf to clear PSE and OSXMMEXCPT
    movl %cr4, %eax
    andl $0x000007cf, %eax
    # or it with 90 to enable PGE and PSE
    orl $0x90, %eax
    movl %eax, %cr4
    # restore calle save 
    popl %eax
    popl %ebx
    popl %edi
    popl %esi
    leave
    ret

# void enablePaging()
# interface: set cr0 to enable paging
# input: NONE
# output: NONE
# side_effects: enable paging
# clobbers: eax
enablePaging:
    # stack set up
    pushl %ebp
    movl %esp, %ebp
    # callee save 
    pushl %esi
    pushl %edi
    pushl %ebx
    pushl %eax
    # enable paging 
    movl %cr0, %eax
    # or with 0x80000000 to enable PG
    orl $0x80000000, %eax
    movl %eax, %cr0
    # restore callee save 
    popl %eax
    popl %ebx
    popl %edi
    popl %esi
    leave
    ret

