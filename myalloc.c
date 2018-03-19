#include "myalloc.h"
void pages_init(){ //chunk mem into pages of 4kb
    mem_block = (char*) memalign(sysconf(_SC_PAGE_SIZE), MEM_SIZE);

    int curr_page = NUM_PROCESSES + KERNEL_MEMORY;
    size_t block_size = SWAP_PAGE - FIRST_USER_PAGE - 2*sizeof(block_meta);

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
    memcpy(&mem_block[SWAP_PAGE - sizeof(block_meta)],
           &temp_block,
           sizeof(block_meta)); 
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
     // Holds current page under inspection.
    block_meta * temp  = (block_meta*) &mem_block[FIRST_USER_PAGE];
    
    // Look for a page that is used by the given tid_req.
    while (temp->next != NULL) {
        if (temp->p_type != UNASSIGNED && temp->tid == tid_req) {
            block_meta * enough_room = find_block_in_page(temp, x);
            if (enough_room != NULL)
                return enough_room;
        }
    }

    return NULL;
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
        printf("physical memory full!\n");
    
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
    return (void *)( (unsigned long)first_fit + sizeof(block_meta) );
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
// Functions for placing and inspecting the top 11 bits of a physical memory
// address into the inverted page table.
//
//------------------------------------------------------------------------------

// Get a virtual address for the given physical address. Behind the scenes this
// function updates the inverted page table to contain the necessary bits from
// the given physical address.
unsigned long get_virtual_address(int num_pages,
                                  int tid, 
                                  unsigned long physical_address) {
    int page = get_page_number_phy(physical_address);
    if (num_pages == 1)
        update_table(1, tid, page, physical_address);
    else
        update_table_multi_page(tid, num_pages, physical_address);

    return build_virtual_address(page, get_physical_offset(physical_address));
}

// Peels of the upper bits of a physical address and stores it at the correct
// position for the given tid and page inside an inverted page table.
void update_table(int has_block_meta, 
                  int tid, 
                  int page, 
                  unsigned long physical_address) {
    int offset = tid * NUM_USER_PAGES + (page * sizeof(int));

    int table_entry = get_upper_phy_mem(physical_address);

    if (has_block_meta > 0)
        table_entry = (1 << 12) + table_entry;
    else
        table_entry = (0 << 12) + table_entry;

    memcpy(&mem_block[offset], &table_entry, sizeof(int));

    return;
}

// Given a physical address where a block is to start and the number of pages
// it extends, update all entries in the inverted page table to contain the
// correct metadata.
void update_table_multi_page(int tid, 
                             int num_pages,
                             unsigned long physical_address) {
    int first_page = get_page_number_phy(physical_address);
    update_table(1, tid, first_page, physical_address);

    int i = first_page + 1;
    for ( ; i < (first_page + num_pages); i++)
        update_table(0, tid, i, physical_address);

    return;
}

// Get the value held in the inverted page table for the given tid and page.
int get_table_entry(int tid, int page) {
    int table_entry;
    int offset = tid * NUM_USER_PAGES + (page * sizeof(int));

    memcpy(&table_entry, &mem_block[offset], sizeof(int));

    return table_entry;
}

// Return the block_meta boolean for the given tid and page. In other words,
// a return value of 1 means that the given page has a block_meta at the
// beginning of the page, 0 if it doesn't.
int contains_block_meta(int tid, int page) {
    int table_entry = get_table_entry(tid, page);
    return table_entry >> 12;
}

// Get the upper_phy value that was stored in the inverted page table for
// the given tid and page.
int get_upper_phy_mem_table(int tid, int page) {
    int table_entry = get_table_entry(tid, page);
    return table_entry & 0xFFF;
}

//------------------------------------------------------------------------------
//
// Functions for adding and demasking parts of a virtual memory address.
//
//------------------------------------------------------------------------------

// Get the page number from the given physical address.
int get_page_number_phy(unsigned long physical_address) {
    physical_address = physical_address -
                       ( (unsigned long) &mem_block
                                       + PAGE_TABLE_SIZE
                                       + (KERNEL_MEMORY * PAGE_SIZE) );
    return physical_address / PAGE_SIZE;
}

// Get the page number from the given virtual address.
int get_page_number_virtual(unsigned long virtual_address) {
    unsigned long mem_base = (unsigned long) &mem_block & 
                             0xFFFFFFFFFFFFF000;
    return (int) (((virtual_address - mem_base) >> 12) & 0x7FF);
}

// Determine the offset inside a page that a memory address maps to.
// This function works to get offset from both virtual addresses and
// physical addresses.
int get_virtual_offset(unsigned long virtual_address) {
    return (int) (virtual_address & 0xFFF);
}

int get_physical_offset(unsigned long physical_address) {
    physical_address = physical_address -
                       ( (unsigned long) &mem_block
                                       + PAGE_TABLE_SIZE
                                       + (KERNEL_MEMORY * PAGE_SIZE) );
    return (int) (physical_address & 0xFFF);
}

// Get the portion of physical memory address to be stored in inverted page
// table.
int get_upper_phy_mem(unsigned long physical_address) {
    physical_address = physical_address -
                       ( (unsigned long) &mem_block
                                       + PAGE_TABLE_SIZE
                                       + (KERNEL_MEMORY * PAGE_SIZE) );
    return (int) ((physical_address >> 12) & 0x7FF);
}

// Build the virtual address to be passed on to user.
unsigned long build_virtual_address(int page, int offset) {
    unsigned long mem_base = (unsigned long) &mem_block & 
                             0xFFFFFFFFFFFFF000;
    return (unsigned long) ((page << 12) + mem_base + offset);
}

int main() {
    pages_init();
    printf("%lu\n", &mem_block);
    printf("%lu\n", build_virtual_address(2, get_physical_offset((unsigned long) &mem_block)));
    printf("%lu\n", get_page_number_virtual(build_virtual_address(2, get_physical_offset((unsigned long) &mem_block))));
    printf("%d\n", get_page_number_phy(6635632 + 4095));
//    update_table(1, 3, 4, (6635632 + 4095));
//    printf("%d\n", get_table_entry(3, 4));
//    printf("%d\n", contains_block_meta(3, 4));
//    printf("%d\n", get_upper_phy_mem_table(3, 4));
    update_table_multi_page(3, 4, ((unsigned long) &mem_block) + FIRST_USER_PAGE + 54097);
    int page = get_page_number_phy(((unsigned long) &mem_block) + FIRST_USER_PAGE + 54097);
    printf("%d\n", get_table_entry(3, page));
    printf("%d\n", get_table_entry(3, page+1));
    printf("%d\n", get_table_entry(3, page+2));
    printf("%d\n", get_table_entry(3, page+3));
    printf("%d\n", get_table_entry(3, page+4));
    printf("%d\n", contains_block_meta(3, page+1));
    printf("%d\n", get_upper_phy_mem_table(3, page+1));

    printf("%d\n", NUM_USER_PAGES);
}

/*
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
//    return 0;
//}
