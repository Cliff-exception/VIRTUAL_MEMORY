#include "myalloc.h"
void pages_init(){ //chunk mem into pages of 4kb

     int curr_page = TABLE_AERA;
     int i = 0;
     while(curr_page < page_num){
     
        page_meta temp_page;
        
        temp_page.start = &mem_block[curr_page * PAGE_SIZE];
        temp_page.start_idx = curr_page * PAGE_SIZE; // debug purpose
        temp_page.tid = -1;
        temp_page.page_id = curr_page; 
        temp_page.free_size = PAGE_SIZE;
        
        //printf("pagenum:%d page addr%d",i,curr_page * PAGE_SIZE);
        page_meta * temp = (page_meta*)&mem_block[i *sizeof(page_meta)];
        memcpy(temp,&temp_page,sizeof(page_meta));
        
        block_meta temp_block;
        //fill the block
            temp_block.istaken = 0;
            temp_block.blk_size = PAGE_SIZE - 2*sizeof(block_meta);
            temp_block.prev = NULL;
            temp_block.next = NULL;
        //
        /*
        printf("init block:\n");
        print_blk_meta(&temp_block);
        */
        memcpy(temp->start,&temp_block,sizeof(block_meta));// copy to lower bound
        temp->blk_list = (block_meta*)temp->start;
        
        memcpy(&mem_block[(curr_page+1) * PAGE_SIZE - sizeof(block_meta)],&temp_block,sizeof(block_meta)); //copy to upper bound
        
            //printf("lower in a page: %d\nhiger in a page: %d\n",curr_page * PAGE_SIZE,(curr_page+1) * PAGE_SIZE - sizeof(block_meta));
            //just some code might not even work
        //printf("\tdata addr%d\n",i,i*sizeof(page_meta)+sizeof(block_meta));
        
        curr_page++;
        i++;
     }
}

page_meta * get_page(int page_id){
    assert(page_id>=TABLE_AERA);
    return (page_meta*)&mem_block[(page_id-TABLE_AERA) * sizeof(page_meta)];
}

page_meta * find_page(int tid){// very bad hashing, subject to change

    return (page_meta*)&mem_block[(tid) * sizeof(page_meta)];
}
void print_page_meta(page_meta * page){
    printf("page id: %d\n",page->page_id);
    if(page->start == &mem_block[page-> page_id* PAGE_SIZE]) printf("%d\t",page->page_id * PAGE_SIZE);
    printf("*start: %x\n",page->start);
    
    printf("free size:%d\ntid: %d\n\n\n\n\n",page->free_size,page->tid);

}
void print_blk_meta(block_meta * blk){

    if(!blk) return;
    printf("size lower: %d\n",blk->blk_size);
    
    printf("size upper: %d\n",((block_meta *)(((long)blk)+blk->blk_size+sizeof(block_meta)))->blk_size);
    
    printf("next block:\n");
    print_blk_meta(blk->next);

}
block_meta * find_block(block_meta * blk_list, size_t x){ //given size x, find the first fit block in a list of blocks
    block_meta * temp = blk_list;

    int i =0;
    while(temp!= NULL){
    
        //printf("block num:%d   size: %d\n",i++,temp->blk_size);
        
        if(!temp->istaken &&
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
    
    page_meta * curr_page = find_page(tid_req);
    curr_page->tid = tid_req;
    
    //printf("page:%d\n",curr_page->page_id);
    block_meta * first_fit = find_block(curr_page->blk_list,x);
    if(!first_fit) {
        printf("page full!\n");
    
        return NULL;
    }
/*  
    printf("before malloc: \n");
    print_blk_meta(first_fit);
*/  
    first_fit -> istaken = 1;
    block_meta * temp_addr = first_fit->next;
    block_meta * old_upper = (block_meta *)((long)(first_fit + 1) + (first_fit->blk_size));
    size_t old_size = first_fit -> blk_size;
    
    
    first_fit -> next = (block_meta*)((long)first_fit + ATTEMP_TAKE);//(x + 2*sizeof(block_meta)); and points to the compliment of taken block
    
    // fill a new block, the compliment of taken block
    block_meta new_block;
    new_block.istaken = 0;
    new_block.blk_size = old_size - ATTEMP_TAKE;
    new_block.next = temp_addr;
    memcpy(first_fit->next, &new_block, sizeof(block_meta)); //set the lower bound for a free block
    
    old_upper->blk_size = new_block.blk_size;       //set the upper bound for a free block
    
    
    first_fit -> blk_size = x;              // set the boundary tag for lower addr
    
    memcpy( (void*)(  ((long)(first_fit+1)) + x ) ,first_fit,sizeof(block_meta));//set the boundray tag for higher addr
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
    
    lower_bound->istaken = 0;
    upper_bound->istaken = 0;
    
    size_t new_size;
    if((upper_bound+1)->istaken == 0 && 
        (((long)(upper_bound+1) - (long)&mem_block)% PAGE_SIZE != 0 )){ //coalescing with a higher free block
        
        //printf("coalescing with a higher free block\n");
        
        new_size = lower_bound -> blk_size + (upper_bound+1)->blk_size + 2*sizeof(block_meta);
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
    
    if((lower_bound-1)->istaken == 0 &&     
            ((((long)lower_bound) - (long)&mem_block)% PAGE_SIZE != 0 )){   //coalescing with a lower free block
    
        //(((long)lower_bound)% PAGE_SIZE != 0 )){ 
        
        //printf("lower_bound addr:%d\n",lower_bound);
    
        //printf("coalescing with a lower free block\n");
        
        new_size =  lower_bound -> blk_size + (lower_bound-1)->blk_size + 2*sizeof(block_meta);
        block_meta * new_lower = (void *) ((long)(lower_bound - 1) - (lower_bound-1)->blk_size);
        
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
    /*
    
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