/* 
 * 
 * filesystem.c - functions to open/close/read files 
 * 
 */

#include "filesystem.h"

/* global variables/structures */


/* init_filesystem
 * DESCRIPTION: initializes boot block and block arrays
 * INPUT: nothing
 * OUTPUT: nothing
 * SIDE EFFECT: 
 */
void init_filesystem(){
    /* init boot block */
    boot_block = (boot_block_t*) filesys_start;

    /* init inode array */
    inodes = (inode_t*) (filesys_start + sizeof(boot_block_t));

    /* init data block array */
    data_blocks = (data_block_t*) (filesys_start + sizeof(boot_block_t) + sizeof(inode_t) * boot_block->inode_count);

}

/* file_open
 * DESCRIPTION: opens the file
 * INPUT: filename: name of file to open
 * OUTPUT: 0 on success, -1 on fail
 * SIDE EFFECT: sets file_pos and inode_num
 */
int32_t file_open(const uint8_t* filename) {
    return 0;
}

/* file_close
 * DESCRIPTION: closes the file
 * INPUT: fd: file descriptor
 * OUTPUT: 0
 * SIDE EFFECT: 
 */
int32_t file_close(int32_t fd) {
    return 0;
}

/* file_read
 * DESCRIPTION: reads nbytes bytes into buf
 * INPUT: fd: file descriptor
 *        buf: buf to copy bytes into
 *        nbytes: number of bytes to copy
 * OUTPUT: number of bytes read
 * SIDE EFFECT: 
 */
int32_t file_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
    /* extract pcb from esp */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);

    /* read data from file and store in buf */
    int32_t num_bytes = read_data(pcb_pointer->fdarray[fd].inode, pcb_pointer->fdarray[fd].file_pos, buf, nbytes);

    /* update file_pos */
    pcb_pointer->fdarray[fd].file_pos = pcb_pointer->fdarray[fd].file_pos + num_bytes;
    
    return num_bytes;
}

/* file_write
 * DESCRIPTION: writes nbytes bytes into file
 * INPUT: fd: file descriptor
 *        buf: buf to write to file
 *        nbytes: number of bytes to copy
 * OUTPUT: -1
 * SIDE EFFECT: 
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* dir_open
 * DESCRIPTION: opens the directory
 * INPUT: filename: name of dir to open
 * OUTPUT: 0 on success, -1 on fail
 * SIDE EFFECT: sets directory index
 */
int32_t dir_open(const uint8_t* filename) {
    return 0;
}

/* dir_close
 * DESCRIPTION: closes the directory
 * INPUT: fd: file descriptor
 * OUTPUT: 0
 * SIDE EFFECT: 
 */
int32_t dir_close(int32_t fd) {
    return 0;
}

/* dir_read
 * DESCRIPTION: reads nbytes bytes into buf
 * INPUT: fd: file descriptor
 *        buf: buf to copy bytes into
 *        nbytes: number of bytes to copy
 * OUTPUT: number of bytes copied
 * SIDE EFFECT: 
 */
int32_t dir_read(int32_t fd, uint8_t* buf, int32_t nbytes) {
    /* extract pcb from esp */
    pcb_t* pcb_pointer = (pcb_t*)((_8MB - t_s[cur_ter].current_running_pid * _8KB - END_OFFSET) & PCB_MASK);
    dentry_t dentry;

    /* read dentry, if fail return 0 */
    if (read_dentry_by_index(pcb_pointer->fdarray[fd].file_pos, &dentry) == -1){
        return 0;
    }

    /* number of bytes copied */
    int32_t num_copied = 0;

    /* index into filename string */
    int32_t filename_index = 0;

    /* copy filename into buffer until either limit is reached or end of filename */
    while (dentry.filename[filename_index] != EOS && num_copied < FILENAME_LENGTH){
        *buf = dentry.filename[filename_index];
        buf++;
        num_copied++;
        filename_index++;
    }

    /* go to next dentry */
    pcb_pointer->fdarray[fd].file_pos++;

    return num_copied;
}

/* dir_write
 * DESCRIPTION: writes nbytes bytes into dir
 * INPUT: fd: file descriptor
 *        buf: buf to write to dir
 *        nbytes: number of bytes to copy
 * OUTPUT: -1
 * SIDE EFFECT: 
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* read_dentry_by_name
 * DESCRIPTION: fills dentry with fields from bootblock corresponding to input name
 * INPUT: fname: name of file
 *        dentry: dentry to fill
 * OUTPUT: 0 on success, -1 on fail
 * SIDE EFFECT: 
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    int i;
    /* search for file name in dentry list, then copy all fields if found, else return -1 */
    for (i = 0; i < boot_block->dir_count; i++){
        if (strncmp((int8_t*)boot_block->dir_entries[i].filename, (int8_t*)fname, strlen((int8_t*)fname)) == 0){
            if (strlen((int8_t*)fname) != strlen((int8_t*)boot_block->dir_entries[i].filename) && strlen((int8_t*)fname) != FILENAME_LENGTH){
                return -1;
            }
            /* copy all fields */
            strcpy((int8_t*)dentry->filename, (int8_t*)boot_block->dir_entries[i].filename);
            dentry->filetype = boot_block->dir_entries[i].filetype;
            dentry->inode_num = boot_block->dir_entries[i].inode_num;
            return 0;
        }
    }
    return -1;
}

/* read_dentry_by_index
 * DESCRIPTION: fills dentry with fields from bootblock corresponding to inode index
 * INPUT: index: inode index
 *        dentry: dentry to fill
 * OUTPUT: 0 on success, -1 on fail
 * SIDE EFFECT: 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry) {
    /* check for valid index */
    if (index >= boot_block->dir_count || index < 0){
        return -1;
    }

    /* copy all fields */
    strcpy((int8_t*)dentry->filename, (int8_t*)boot_block->dir_entries[index].filename);
    dentry->filetype = boot_block->dir_entries[index].filetype;
    dentry->inode_num = boot_block->dir_entries[index].inode_num;
    return 0;
}

/* read_data
 * DESCRIPTION: reads length bytes into buf
 * INPUT: inode: inode number
 *        offset: offset into file to start reading from
 *        buf: buf to copy bytes into
 *        nbytes: number of bytes to copy
 * OUTPUT: number of bytes read
 * SIDE EFFECT: 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    /* check for valid inode index */
    if (inode >= boot_block->inode_count || inode < 0){
        return -1;
    }

    /* inode for given inode index */
    inode_t file_inode = inodes[inode];

    /* data block index in inode */
    int32_t data_block_idx = offset / BLOCK_SIZE;

    /* pointer to current block in data block array*/
    data_block_t* block = &data_blocks[file_inode.data_block_num[data_block_idx]];

    /* offset from start of block */
    int32_t data_offset = offset % BLOCK_SIZE; 

    /* number of bytes copied already */
    int num_copied = 0;
    
    /* loop to copy bytes, stop when num_copied = length */
    while (num_copied < length){
        
        /* if end of file, break */
        if (offset + num_copied >= file_inode.length){
            break;
        }

        /* if bytes go past current block, go to next block */
        if (data_offset == BLOCK_SIZE){
            data_offset = 0;
            block = &data_blocks[file_inode.data_block_num[data_block_idx + 1]];
        }

        /* copy from block to buffer, increment indices */
        *buf = block->byte[data_offset];
        buf++;
        data_offset++;
        num_copied++;
    }
    return num_copied;
}

/* stdout_write
 * DESCRIPTION: do nothing
 * INPUT: fd: file descriptor
 *        buf: buf to copy bytes into
 *        nbytes: number of bytes to copy
 * OUTPUT: -1
 * SIDE EFFECT: 
 */
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* stdout_read
 * DESCRIPTION: do nothing
 * INPUT: fd: file descriptor
 *        buf: buf to copy bytes into
 *        nbytes: number of bytes to copy
 * OUTPUT: -1
 * SIDE EFFECT: 
 */
int32_t stdout_read(int32_t fd, uint8_t* buf, int32_t nbytes){
    return -1;
}


