#ifndef MYALLOC_H
#define MYALLOC_H



#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#include <stdio.h>

#define MEM_SIZE (8*1024*1024)
#define PAGE_SIZE (4*1024)
#define NUM_PROCESSES (100) // Needs to be even to prevent alignment problems.
#define PAGE_TABLE_SIZE (2048 * NUM_PROCESSES)
#define KERNEL_MEMORY 32
#define FIRST_USER_PAGE (PAGE_TABLE_SIZE + (KERNEL_MEMORY * PAGE_SIZE))
#define NUM_USER_PAGES ((MEM_SIZE - (KERNEL_MEMORY * PAGE_SIZE) \
                                  - (PAGE_TABLE_SIZE))          \
                         / PAGE_SIZE)
#define SWAP_PAGE (FIRST_USER_PAGE + (NUM_USER_PAGES * PAGE_SIZE))
#define LIBRARYREQ -1
//#define THREADREQ 1
#define SHARED_PAGE 4

#define ATTEMP_TAKE (x+2*sizeof(block_meta))
//static char mem_block[MEM_SIZE];
static char * mem_block;
static int page_num = MEM_SIZE/PAGE_SIZE; //should be 2048 without tune it


typedef enum Page_Type {UNASSIGNED, THREAD, DATA} page_type;

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
    size_t blk_size;
    size_t free_size;
    int tid;

    struct blk * prev;
    struct blk * next;
}block_meta;

void pages_init();
//page_meta * get_page(int page_id);
//page_meta * find_page(int tid);
//void print_page_meta(page_meta * page);
void print_blk_meta(block_meta * blk);
block_meta * find_block(int tid_req, size_t x);
block_meta * find_block_in_page(block_meta * blk_list, size_t x);
void * myallocate(size_t x, char * file, int linenum, int tid_req);
void mydeallocate(void * ptr, char * file, int linenum, int tid_req);

int get_active_tid(int page);
void swap_pages(int tid, int current_page, int target_page);

unsigned long get_virtual_address(int num_pages,
                                  int tid, 
                                  unsigned long physical_address);
void update_table_entry(int tid, int page, int new_page);
void update_table_address(int has_block_meta, 
                          int tid, 
                          int page, 
                          unsigned long physical_address);
void update_table_multi_page(int tid, 
                             int num_pages,
                             unsigned long physical_address);
int get_table_entry(int tid, int page);
int contains_block_meta(int tid, int page);
int get_upper_phy_mem_table(int tid, int page);

int get_page_number_phy(unsigned long physical_address);
int get_virtual_offset(unsigned long virtual_address);
int get_physical_offset(unsigned long physical_address);
int get_upper_phy_mem(unsigned long physical_address);
int get_page_number_virtual(unsigned long virtual_address);
unsigned long build_virtual_address(int page, int offset);

#endif
