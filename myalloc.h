#ifndef MYALLOC_H
#define MYALLOC_H

#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include<unistd.h>
#include<sys/mman.h>
#include<stddef.h>

#include <stdlib.h>
#include <stdio.h>

#define malloc(x) myallocate(x, __FILE__, __LINE__, get_curr_tid());
#define free(x) mydeallocate(x, __FILE__, __LINE__, get_curr_tid());

#define MEM_SIZE (8*1024*1024)
#define PAGE_SIZE (4*1024)
#define NUM_PROCESSES (64) // Needs to be even to prevent alignment problems.
#define OUT_OF_BOUNDS (-1)
#define UNASSIGNED_IN_TABLE (-2)
#define PAGE_TABLE_SIZE (2048 * (NUM_PROCESSES + 4) * 4) // Last page for used pages.
//#define KERNEL_MEMORY 32
//#define FIRST_USER_PAGE (PAGE_TABLE_SIZE + (KERNEL_MEMORY * PAGE_SIZE))
#define FIRST_USER_PAGE (PAGE_TABLE_SIZE)
//#define NUM_USER_PAGES ((MEM_SIZE - (KERNEL_MEMORY * PAGE_SIZE) \
//                                  - (PAGE_TABLE_SIZE))          \
//                         / PAGE_SIZE)
#define NUM_USER_PAGES ((MEM_SIZE - (PAGE_TABLE_SIZE)) / PAGE_SIZE)
#define SWAP_PAGE (FIRST_USER_PAGE + (NUM_USER_PAGES * PAGE_SIZE))
#define LIBRARYREQ -1
//#define THREADREQ 1
#define SHARED_PAGE 4

#define ATTEMP_TAKE (x+2*sizeof(block_meta))
//static char mem_block[MEM_SIZE];
static char * mem_block;
static int page_num = MEM_SIZE/PAGE_SIZE; //should be 2048 without tune it


typedef enum Page_Type {UNASSIGNED, THREAD, DATA} page_type;

int swap_file_descriptor; // file descriptor for the swap_file
/* Tyson - Commented out ken's structs in case changes fail.
typedef struct blk{
    int istaken;
    size_t blk_size;

    struct blk * prev;
    struct blk * next;
}block_meta;

typedef struct _PageData{
    char * start;//Stores where the memory is supposed to start
    int start_idx; //for debug purpose
    int tid; //free is -1
    int page_id;
    int free_size;
    //int conti; //1 contiguous; 0 not
    struct blk * blk_list;//implicit free list
    //struct _PageData *next;
}page_meta;
*/

typedef struct blk{
    page_type p_type;
    size_t blk_size; // ghost
    size_t free_size;
    int tid;		//ghost

    struct blk * prev;
    struct blk * next;
}block_meta;

void pages_init();
//page_meta * get_page(int page_id);
//page_meta * find_page(int tid);
//void print_page_meta(page_meta * page);
void print_blk_meta(block_meta * blk);
block_meta * init_block_meta_page(int tid_req, int x, block_meta * prev, block_meta * next);
block_meta * init_block_meta_page_zero(int tid_req);
block_meta * find_block(int tid_req, size_t x);
block_meta * find_block_in_page(block_meta * blk_list, size_t x);
void * myallocate(size_t x, char * file, int linenum, int tid_req);
void mydeallocate(void * ptr, char * file, int linenum, int tid_req);

int get_active_tid(int page);
void swap_pages(int in_pos_page, int out_tid, int out_pos_page);
void swap( int mem_page, int tid );
void memory_protect_page(int page);
void memory_unprotect_page(int page);
void protect_all_tid_pages(int tid);
void unprotect_all_tid_pages(int tid);
int get_table_offset(int tid_req, int page);

int create_table_entry(int location, int page);
int get_location(int tid, int page);
int get_page_from_table(int tid, int page);
int is_in_memory(int tid, int page);
int is_in_upper_swap(int tid, int page);
int is_in_lower_swap(int tid, int page);
void update_table_entry(int tid, int page, int new_page, int location);
int get_table_entry(int tid, int page);
int contains_block_meta(int tid, int page);

int get_note_page_offset(int page);
void note_page_used(int page);
void note_page_unused(int page);
int get_unused_page();

int get_note_page_offset_file(int portion, int page);
void note_page_used_file(int page);
void note_page_unused_file(int page);
int get_unused_page_file();

int get_page_number_real_phy(unsigned long physical_address);
int get_page_number_phy(unsigned long physical_address);
int get_virtual_offset(unsigned long virtual_address);
int get_physical_offset(unsigned long physical_address);
int get_upper_phy_mem(unsigned long physical_address);
int get_page_number_virtual(unsigned long virtual_address);
unsigned long build_virtual_address(int page, int offset);
unsigned long safely_align_block(unsigned long phy_addr);


int swap_space_init();
void evict_page ( unsigned long address, int swap_file_offset );
int get_from_swap ( unsigned long to_mem_offset, int out_swap_offset);
void swap( int mem_page, int tid); 

#endif
