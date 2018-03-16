#ifndef MYALLOC_H
#define MYALLOC_H



#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
      /* #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h> */

#include <stdio.h>

#define MEM_SIZE (8*1024*1024)
#define PAGE_SIZE (4*1024)
#define TABLE_AERA 32
#define LIBRARYREQ -1
//#define THREADREQ 1
#define SHARED_PAGE 4

#define ATTEMP_TAKE (x+2*sizeof(block_meta))
static char mem_block[MEM_SIZE];
static int page_num = MEM_SIZE/PAGE_SIZE; //should be 2048 without tune it

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


void pages_init();
page_meta * get_page(int page_id);
page_meta * find_page(int tid);
void print_page_meta(page_meta * page);
void print_blk_meta(block_meta * blk);
block_meta * find_block(block_meta * blk_list, size_t x);
void * myallocate(size_t x, char * file, int linenum, int tid_req);
void mydeallocate(void * ptr, char * file, int linenum, int tid_req);

#endif