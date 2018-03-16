#include "myalloc.h"
void pages_init(){ //chunk mem into pages of 4kb

    int curr_page = KERNEL_MEMORY;
    size_t block_size = PAGE_SIZE - 2*sizeof(block_meta);

    while(curr_page < page_num){
     
        block_meta temp_block;
        temp_block.p_type = UNASSIGNED;
        temp_block.blk_size = block_size;
        temp_block.free_size = block_size;
        temp_block.tid = -1;
        temp_block.prev = NULL;
        temp_block.next = NULL;
        
        // copy to lower bound
        memcpy(&mem_block[curr_page * PAGE_SIZE],
               &temp_block,
               sizeof(block_meta));
        
        //copy to upper bound
        memcpy(&mem_block[(curr_page+1) * PAGE_SIZE - sizeof(block_meta)],
               &temp_block,
               sizeof(block_meta)); 
        
        curr_page++;
    }

    // Ensure that the first user block is of type THREAD, it will be used for
    // dereferencing memory addresses.
    block_meta * temp_block = (block_meta*) &mem_block[MAIN_PAGE];
    temp_block->p_type = THREAD;
}

/* Tyson - Commenting out for time being, need to fix to work with memory mapping.
page_meta * get_page(int page_id){
    assert(page_id>=KERNEL_MEMORY);
    return (page_meta*)&mem_block[(page_id-KERNEL_MEMORY) * sizeof(page_meta)];
}

page_meta * find_page(int tid){// very bad hashing, subject to change

    return (page_meta*)&mem_block[(tid) * sizeof(page_meta)];
}
void print_page_meta(page_meta * page){
    printf("page id: %d\n",page->page_id);
    if(page->start == &mem_block[page-> page_id* PAGE_SIZE]) printf("%d\t",page->page_id * PAGE_SIZE);
    printf("*start: %x\n",page->start);
    
    printf("free size:%d\ntid: %d\n\n\n\n\n",page->free_size,page->tid);

} */

void print_blk_meta(block_meta * blk){

    if(!blk) return;
    printf("size lower: %d\n",blk->blk_size);
    
    printf("size upper: %d\n",
           ((block_meta *)(((long)blk) + 
                           blk->blk_size +
                           sizeof(block_meta)))->blk_size);
    
    printf("next block:\n");
    print_blk_meta(blk->next);

}

block_meta * find_block(int tid_req, size_t x) {
    // If x is larger than size of memory in a page, at this point we can't
    // hold it and must reject.
    if (x > (PAGE_SIZE - 2 * sizeof(block_meta)))
        return NULL;

    block_meta * temp  = NULL;  // Holds current page under inspection.
    block_meta * first = NULL;  // First UNASSIGNED page found just in case.
    
    // Look for a page that is used by the given tid_req.
    int i = 0;
    for ( ; i < USER_PAGES; i++) {
        temp = (block_meta*) &mem_block[(MAIN_PAGE + PAGE_SIZE) + 
                                        (i * PAGE_SIZE)];
        if (first == NULL && temp->p_type == UNASSIGNED)
            first = temp;
        else if (temp->p_type != UNASSIGNED && temp->tid == tid_req) {
            block_meta * enough_room = find_block_in_page(temp, x);
            if (enough_room != NULL)
                return enough_room;
        }
    }

    // At this point, all pages with given tid = tid_req don't have enough
    // room for storing x bits. Using the first found UNASSIGNED page.
    return first;
}

// given size x, find the first fit block in a list of blocks
block_meta * find_block_in_page(block_meta * blk_list, size_t x){ 
    block_meta * temp = blk_list;

    int i =0;
    while(temp!= NULL){
    
        //printf("block num:%d   size: %d\n",i++,temp->blk_size);
        
        if(temp->p_type == UNASSIGNED &&
            (temp->blk_size >= ATTEMP_TAKE)){
            return temp;
        }
        temp = temp->next;
    }
    
    return NULL;
}

//void * myallocate(size_t x, __FILE__, __LINE__, THREADREQ){
void * myallocate(size_t x, char * file, int linenum, int tid_req){
    
    assert(x>0);
    if(tid_req == LIBRARYREQ){//maybe later
    
        return NULL;
    }
    /*
    page_meta * curr_page = find_page(tid_req);
    curr_page->tid = tid_req;
    */
    
    //printf("page:%d\n",curr_page->page_id);
    //block_meta * first_fit = find_block(curr_page->blk_list,x);
    
    block_meta * first_fit = find_block(tid_req, x);
    if(!first_fit) {
        printf("page full!\n");
    
        return NULL;
    }
/*  
    printf("before malloc: \n");
    print_blk_meta(first_fit);
*/  
    first_fit->p_type = DATA;
    block_meta * temp_addr = first_fit->next;
    block_meta * old_upper = (block_meta *)((long)(first_fit + 1) + 
                                            (first_fit->blk_size));
    size_t old_size = first_fit -> blk_size;
    
    //(x + 2*sizeof(block_meta)); and points to the compliment of taken block
    first_fit -> next = (block_meta*)((long)first_fit + ATTEMP_TAKE);
    
    // fill a new block, the compliment of taken block
    block_meta new_block;
    new_block.p_type = UNASSIGNED;
    new_block.blk_size = old_size - ATTEMP_TAKE;
    new_block.next = temp_addr;

    //set the lower bound for a free block
    memcpy(first_fit->next, &new_block, sizeof(block_meta)); 
    
    //set the upper bound for a free block
    old_upper->blk_size = new_block.blk_size;       
    
    // set the boundary tag for lower addr 
    first_fit -> blk_size = x;            
    
    //set the boundray tag for higher addr
    memcpy( (void*) (( (long)(first_fit+1) ) + x), first_fit,sizeof(block_meta));
    // ugly & very subject to bug
/*
    printf("after malloc: \n");
    print_blk_meta(first_fit);
    
    printf("\n\n\n\n");
*/
    
    return (void *)( (long)first_fit + sizeof(block_meta) );
}

//mydeallocate(x, __FILE__, __LINE__, THREADREQ)
void mydeallocate(void * ptr, char * file, int linenum, int tid_req){
    //assuming inputs are valid
    block_meta * lower_bound = (block_meta *)((long)ptr - sizeof(block_meta));
    block_meta * upper_bound = (block_meta *)((long)ptr + lower_bound->blk_size);
    
    /*
    printf("before free the block:\n");
    print_blk_meta(lower_bound);
    */
    
    lower_bound->p_type = UNASSIGNED;
    upper_bound->p_type = UNASSIGNED;
    
    size_t new_size;
    if((upper_bound+1)->p_type == UNASSIGNED && 
        (((long)(upper_bound+1) - (long)&mem_block)% PAGE_SIZE != 0 )){ //coalescing with a higher free block
        
        //printf("coalescing with a higher free block\n");
        
        new_size = lower_bound -> blk_size + 
                                  (upper_bound+1)->blk_size + 
                                  2*sizeof(block_meta);
        block_meta * new_upper = (void *) ((long)(lower_bound + 1) + new_size);
        
        lower_bound -> blk_size = new_size;
        new_upper   -> blk_size = new_size;
        lower_bound -> next     = new_upper->next;
        
        upper_bound = new_upper; // make a block logically contiguous
        
        /*
        printf("after coalescing:\n");
        print_blk_meta(lower_bound);
        */
    }
    
    if((lower_bound-1)->p_type == UNASSIGNED &&     
            ((((long)lower_bound) - (long)&mem_block)% PAGE_SIZE != 0 )){   //coalescing with a lower free block
    
        //(((long)lower_bound)% PAGE_SIZE != 0 )){ 
        
        //printf("lower_bound addr:%d\n",lower_bound);
    
        //printf("coalescing with a lower free block\n");
        
        new_size =  lower_bound->blk_size + 
                    (lower_bound-1)->blk_size + 
                    2*sizeof(block_meta);
        block_meta * new_lower = (void *) ((long)(lower_bound - 1) - 
                                           (lower_bound-1)->blk_size);
        
        //printf("new size %d = %d + %d + 2*meta:%d\n",new_size, lower_bound -> blk_size ,(lower_bound-1)->blk_size,2*sizeof(block_meta));
        upper_bound -> blk_size = new_size;
        new_lower   -> blk_size = new_size;
        new_lower   -> next     = upper_bound->next;
        /*
        printf("after coalescing:\n");
        print_blk_meta(new_lower);
        */
    }
    
}

//------------------------------------------------------------------------------
//
// Functions for demasking parts of a virtual memory address.
//
//------------------------------------------------------------------------------

// Get the thread id for a given memory address.
int get_tid(unsigned long memory_address) {
    return memory_address = (int) ((memory_address >> 24) & 0xFFFFFFFF);
}

// Determine the offset from the main page which maps to the corresponding
// thread page.
int get_thread_page_identifier(unsigned long memory_address) {
    return memory_address = (int) ((memory_address >> 18) & 0x3F);
}

// Determine the offset for the pointer in a thread page that leads to the
// correct user data page.
int get_thread_page_map(unsigned long memory_address) {
    return memory_address = (int) ((memory_address >> 12) & 0x3F);
}

// Determine the offset inside a page that a memory address maps to.
int get_offset(unsigned long memory_address) {
    return (int) (memory_address & 0xFFF);
}

int main(){
    //num_page = MEM_SIZE/ (sizeof(page_meta)+ PAGE_SIZE) ; AND TAKE THE FLOOR
    pages_init();
    char * test0 = (char *) myallocate(15,__FILE__,__LINE__,2);
    
    strcpy(test0, "hello world!");
    printf("%s\n",test0);
    
    
    
    
    char * test1 = (char *) myallocate(15,__FILE__,__LINE__,2);
    char * test2 = (char *) myallocate(15,__FILE__,__LINE__,2);
    
    printf("malloced******************\n\n\n\n");
    
    
    printf("first free:\n");
    mydeallocate(test0,__FILE__,__LINE__,2);
    
    printf("2nd free:\n");
    mydeallocate(test1,__FILE__,__LINE__,2);
    
    printf("3rd free:\n");
    mydeallocate(test2,__FILE__,__LINE__,2);
    
    
    printf("******************\n\n\n\n");
    
    //char * test3 = (char *) myallocate(15,__FILE__,__LINE__,2);
    
    /*
    int i = 32;
    while(i<2013){
        print_page_meta(get_page(i));
        i++;
    }
    */
    return 0;
}
