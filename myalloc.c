#include "myalloc.h"
#include "my_pthread_t.h"

static void handler(int sig, siginfo_t *si, void *unused) {
//    printf("Got SIGSEGV at address: 0x%lx\n",(long) si->si_addr);
    int page = get_page_number_real_phy((unsigned long) si->si_addr);
    printf("page: %d\n", page);
    memory_unprotect_page(page);
    int tid = get_curr_tid();
    swap_pages(tid, page, tid);
}

void pages_init(){
    // Initialize signal handler.
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;

    if (sigaction(SIGSEGV, &sa, NULL) == -1)
    {
        printf("Fatal error setting up signal handler\n");
        exit(EXIT_FAILURE);    //explode!
    }

    // Initialize memory.
    mem_block = (char*) memalign(sysconf(_SC_PAGE_SIZE), MEM_SIZE);

    // Update inverted page table to show that no pages have been assigned.
    int out_of_bounds = OUT_OF_BOUNDS;
    int i = 0;
    for ( ; i < NUM_PROCESSES; i++)
        memcpy(&mem_block[get_table_offset(i, 0)], &out_of_bounds, sizeof(int));

    int unassigned = UNASSIGNED_IN_TABLE;
    i = 0;
    int j;
    for ( ; i < NUM_PROCESSES; i++) {
        j = 1;
        for ( ; j < NUM_USER_PAGES; j++) {
            memcpy(&mem_block[get_table_offset(i, j)],
                   &unassigned,
                   sizeof(int));
        }
    }

    return;
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
    if(page->start == &mem_block[page-> page_id* PAGE_SIZE]) 
        printf("%d\t",page->page_id * PAGE_SIZE);
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

block_meta * init_block_meta_page(int tid_req, int page, block_meta * prev, block_meta * next) {
    size_t block_size = SWAP_PAGE -
                        (FIRST_USER_PAGE + (page+1) * PAGE_SIZE);

    block_meta temp_block;
    temp_block.p_type = UNASSIGNED;
    temp_block.blk_size = block_size;
    temp_block.free_size = block_size;
    temp_block.tid = tid_req;
    temp_block.prev = prev;
    temp_block.next = next;

    block_meta * address = (block_meta *) 
        &mem_block[FIRST_USER_PAGE + ((page+1) * PAGE_SIZE) - sizeof(block_meta)];

    memcpy((void*)address, &temp_block, sizeof(block_meta));

    return address;
}

block_meta * init_block_meta_page_zero(int tid_req) {
    size_t block_size = SWAP_PAGE - FIRST_USER_PAGE - sizeof(block_meta);

    block_meta temp_block;
    temp_block.p_type = UNASSIGNED;
    temp_block.blk_size = block_size;
    temp_block.free_size = block_size;
    temp_block.tid = tid_req;
    temp_block.prev = NULL;
    temp_block.next = NULL;

    block_meta * address = (block_meta *) &mem_block[FIRST_USER_PAGE];

    memcpy((void*)address, &temp_block, sizeof(block_meta));

    return address;
}


block_meta * find_block(int tid_req, size_t x) {
    // TODO: Make sure that if found block has an empty page, note empty page as used.

    block_meta * b_meta = (block_meta *) &mem_block[FIRST_USER_PAGE];
    //
    block_meta * next_meta;
    int max_page;
    
    
    
    //
    int current_loc_page_zero = get_table_entry(tid_req, 0);
    if (current_loc_page_zero == OUT_OF_BOUNDS) {
    
        b_meta = init_block_meta_page_zero(tid_req);
        
        next_meta = (block_meta *) &mem_block[FIRST_USER_PAGE + sizeof(block_meta) + x];
        max_page = ((unsigned long)(next_meta) + sizeof(block_meta) - (unsigned long)&mem_block[FIRST_USER_PAGE]) / PAGE_SIZE;
        
        b_meta->next = next_meta;
        next_meta->prev = b_meta;
        
        b_meta->free_size = 0;
        next_meta->free_size = next_meta->prev->free_size - x - sizeof(block_meta); //maybe the source of bug
    
        b_meta->p_type = DATA;
        next_meta->p_type = UNASSIGNED;
    
        b_meta->tid = tid_req;
        next_meta->tid = tid_req; 
        /*
        int unused_page = get_unused_page();

        if (unused_page != 0) {
            note_page_used(unused_page);
            swap_pages(0, tid_req, unused_page);
        }
        note_page_used(0);
    */
    
    
    
        int i = 0;
        int unused_page;
        while(i<max_page){
            unused_page= get_unused_page();
            swap_pages(i, tid_req, unused_page);
            i++;
        }
    
       
        return b_meta;
    }

    int active_tid = get_active_tid(0);
//    if (active_tid != tid_req)
//        swap_pages(tid_req, 0, get_upper_phy_mem_table(active_tid, 0));

    while (b_meta->free_size < (sizeof(block_meta) + x)) {
        if (b_meta->next == NULL)
            return NULL;

        b_meta = b_meta->next;
    }

    return b_meta;
}

void protect_all_tid_pages(int tid){
    /*
    int curr_page = 0;
    while(curr_page < NUM_USER_PAGES && 
        UNASSIGNED_IN_TABLE != get_upper_phy_mem_table(tid, curr_page)){        
        
        memory_protect_page(curr_page);
        curr_page++;
    }
    */
    
    int curr_page = 0, prot_page;
    while(curr_page < NUM_USER_PAGES){
        prot_page = get_upper_phy_mem_table(tid, curr_page);
        memory_protect_page(prot_page);
        curr_page++;
    }
}

void unprotect_all_tid_pages(int tid){
    /*
    int curr_page = 0;
    
    while(curr_page < NUM_USER_PAGES && 
        UNASSIGNED_IN_TABLE != get_upper_phy_mem_table(tid, curr_page)){        
        
        memory_unprotect_page(curr_page);
        curr_page++;
    }
    */
    int curr_page = 0, prot_page;
    while(curr_page < NUM_USER_PAGES){
        prot_page = get_upper_phy_mem_table(tid, curr_page);
        memory_unprotect_page(prot_page);
        curr_page++;
    }
    
}

void swap_protection(int out_tid,int in_tid){
    protect_all_tid_pages(out_tid);
    unprotect_all_tid_pages(in_tid);
}
/* UPDATE: No longer assign block met inside a single page.
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
*/

//void * myallocate(size_t x, __FILE__, __LINE__, THREADREQ){
void * myallocate(size_t x, char * file, int linenum, int tid_req){
    
    assert(x>0);
    if(tid_req == LIBRARYREQ ){  // maybe later
       
        return NULL;   
    } 
    if(tid_req > NUM_PROCESSES ||
       tid_req < 0 ) {
    
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
    //block_meta * old_upper = (block_meta *)((long)(first_fit + 1) + 
    //                                        (first_fit->blk_size));
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
    //old_upper->blk_size = new_block.blk_size;       
    
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

    return (void *)( 
        get_virtual_address(1, tid_req, ((unsigned long) first_fit) + sizeof(block_meta) ));
        
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
    //coalescing with a higher free block
    if((upper_bound+1)->p_type == UNASSIGNED && 
        (((long)(upper_bound+1) - (long)&mem_block)% PAGE_SIZE != 0 )){
        
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
    
    //coalescing with a lower free block
    if((lower_bound-1)->p_type == UNASSIGNED &&     
            ((((long)lower_bound) - (long)&mem_block)% PAGE_SIZE != 0 )){
    
        //(((long)lower_bound)% PAGE_SIZE != 0 )){ 
        
        //printf("lower_bound addr:%d\n",lower_bound);
    
        //printf("coalescing with a lower free block\n");
        
        new_size =  lower_bound->blk_size + 
                    (lower_bound-1)->blk_size + 
                    2*sizeof(block_meta);
        block_meta * new_lower = (void *) ((long)(lower_bound - 1) - 
                                           (lower_bound-1)->blk_size);
        
//        printf("new size %d = %d + %d + 2*meta:%d\n",new_size, 
//                                                     lower_bound -> blk_size ,
//                                                     (lower_bound-1)->blk_size,
//                                                     2*sizeof(block_meta));
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
// Functions that handle the swapping of pages in memory as well as updating
// their corresponding entries in the inverted page table.
//
//------------------------------------------------------------------------------

// For a given page number, determine the tid that currently owns the data
// within this page. A return value of -1 means that this page is currently
// free.
int get_active_tid(int page) {
    int i = 0;
    int current_page;
    for ( ; i < NUM_PROCESSES; i++) {
        current_page = get_upper_phy_mem_table(i, page);
        //printf("%d:%d\n", page, current_page);
        if (current_page == page)
            return i;
    }

    return -1;
}

void swap_pages(int in_pos_page, int out_tid, int out_pos_page) {

    memory_unprotect_page(in_pos_page);
    int in_offset = FIRST_USER_PAGE + in_pos_page * PAGE_SIZE;
    int out_offset = FIRST_USER_PAGE + out_pos_page * PAGE_SIZE;

    // Swap pages within memory utilizing swap page location as pivot.
    memcpy(&mem_block[SWAP_PAGE], &mem_block[in_offset], PAGE_SIZE);
    memcpy(&mem_block[in_offset], &mem_block[out_offset], PAGE_SIZE);
    memcpy(&mem_block[out_offset], &mem_block[SWAP_PAGE], PAGE_SIZE);

    memory_protect_page(out_pos_page);
    int in_tid = get_active_tid(in_pos_page);

    update_table_entry(in_tid, in_pos_page, out_pos_page);
    update_table_entry(out_tid, in_pos_page, in_pos_page);

    return;
}

void memory_protect_page(int page) {
    unsigned long address = (unsigned long)&mem_block[FIRST_USER_PAGE + (page * PAGE_SIZE)];
    mprotect((void*) address, PAGE_SIZE, PROT_NONE);
}

void memory_unprotect_page(int page) {
    unsigned long address = (unsigned long)&mem_block[FIRST_USER_PAGE + (page
        * PAGE_SIZE)];
    mprotect((void*) address, PAGE_SIZE, PROT_READ | PROT_WRITE);
}

int get_table_offset(int tid_req, int page) {
    int offset = (tid_req * NUM_USER_PAGES * sizeof(int) + page * sizeof(int));
    return offset;
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
    int page = get_page_number_real_phy(physical_address);
    if (num_pages == 1)
        update_table_address(1, tid, page, physical_address);
    else
        update_table_multi_page(tid, num_pages, physical_address);

    return build_virtual_address(page, get_physical_offset(physical_address));
}

// Update the page that is stored for a given tid inside the inverted page
// table.
void update_table_entry(int tid, int page, int new_page) {
    int offset = get_table_offset(tid, page);
//    int offset = tid * NUM_USER_PAGES + (page * sizeof(int));

    int has_block_meta  = contains_block_meta(tid, page);

    if (has_block_meta > 0)
        new_page = (1 << 12) + new_page;

    memcpy(&mem_block[offset], &new_page, sizeof(int));

    return;
}



// Peels off the upper bits of a physical address and stores it at the correct
// position for the given tid and page inside an inverted page table.
void update_table_address(int has_block_meta, 
                          int tid, 
                          int page, 
                          unsigned long physical_address) {
    note_page_used(page);

    //int offset = tid * NUM_USER_PAGES + (page * sizeof(int));
    int offset = get_table_offset(tid, page);

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
    int page = get_page_number_real_phy(physical_address);
    printf("PAGE: %d\n", page);

    int first_page = get_page_number_real_phy(physical_address);
    update_table_address(1, tid, first_page, physical_address);

    physical_address += PAGE_SIZE;
    int i = first_page + 1;
    for ( ; i < (first_page + num_pages); i++) {
        update_table_address(0, tid, i, physical_address);
        physical_address += PAGE_SIZE;
    }

    return;
}

// Get the value held in the inverted page table for the given tid and page.
int get_table_entry(int tid, int page) {
    int table_entry;
    int offset = get_table_offset(tid, page);

    memcpy(&table_entry, &mem_block[offset], sizeof(int));

    //printf("TABLE ENTRY: %d\n", table_entry);

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
// Functions that allow keeping track of which pages are currently being used
// or free.
//
//------------------------------------------------------------------------------

int get_note_page_offset(int page) {
    return get_table_offset(NUM_PROCESSES, page);
}

void note_page_used(int page) {
    int offset = get_note_page_offset(page);
    int used = 1;
    memcpy(&mem_block[offset], &used, sizeof(int));
}

void note_page_unused(int page) {
    int offset = get_note_page_offset(page);
    int used = 0;
    memcpy(&mem_block[offset], &used, sizeof(int));
}

int get_unused_page() {
    int i = 0;
    int page_used;
    for ( ; i < NUM_USER_PAGES; i++) {
        memcpy(&page_used, &mem_block[get_note_page_offset(i)], sizeof(int));
        if (!page_used)
            return i;
    }

    return -1;
}

//------------------------------------------------------------------------------
//
// Functions for adding and demasking parts of a virtual memory address.
//
//------------------------------------------------------------------------------

// Get the page number from the given physical address.
int get_page_number_real_phy(unsigned long physical_address) {
    physical_address = ((physical_address - 
                        (unsigned long) mem_block) - 
                        FIRST_USER_PAGE);
    int page_num = physical_address / PAGE_SIZE;
    return physical_address / PAGE_SIZE;
}

/*
// Get the page number from the given physical address.
int get_page_number_phy(unsigned long physical_address) {
    physical_address = physical_address -
                       ( (unsigned long) &mem_block
                                       + PAGE_TABLE_SIZE
                                       + (KERNEL_MEMORY * PAGE_SIZE) );
    page_num = physical_address / PAGE_SIZE;
    return physical_address / PAGE_SIZE;
}
*/

// Get the page number from the given virtual address.
int get_page_number_virtual(unsigned long virtual_address) {
    return (int) (virtual_address - 
                   (unsigned long) &mem_block[FIRST_USER_PAGE]) / 
                 PAGE_SIZE;
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
    physical_address = ((physical_address -
                        (unsigned long) mem_block) -
                        FIRST_USER_PAGE);
//                       ( (unsigned long) &mem_block
//                                      + PAGE_TABLE_SIZE
//                                      + (KERNEL_MEMORY * PAGE_SIZE) );
    return (int) ((physical_address >> 12) & 0x7FF);
}

// Build the virtual address to be passed on to user.
unsigned long build_virtual_address(int page, int offset) {
    return 
        (unsigned long) &mem_block[FIRST_USER_PAGE + page * PAGE_SIZE + offset];
}

unsigned long safely_align_block(unsigned long phy_addr){
    unsigned long page_bound = (unsigned long)&mem_block + (get_page_number_real_phy(phy_addr)+1) * PAGE_SIZE;
    unsigned long left = phy_addr, right = left+ sizeof(block_meta);
    
    if(left < page_bound && right > page_bound){
        //shift
        return page_bound;
    
    }
    
    return phy_addr;
}

int main() {
    pages_init();

    int * my_nums1 = (int*) myallocate(20 * sizeof(int), NULL, 0, 5);

    my_nums1[0] = 5;
    my_nums1[1] = 2;
    printf("NUM 1: %d\n", my_nums1[0]);
    printf("NUM 2: %d\n", my_nums1[1]);

    printf("\nSwapping Pages\n\n");
    swap_pages(0, 6, 2);

    printf("NUM 1: %d\n", my_nums1[0]);
    printf("NUM 2: %d\n", my_nums1[1]);

    printf("\nSwapping Pages\n\n");
    swap_pages(0, 5, 2);

    printf("NUM 1: %d\n", my_nums1[0]);
    printf("NUM 2: %d\n", my_nums1[1]);



    
    return 0;
}


/*
int main() {
    pages_init();
    unsigned long test_address = ((unsigned long) &mem_block) + FIRST_USER_PAGE + 54097;
    int page = get_page_number_phy(((unsigned long) &mem_block) + FIRST_USER_PAGE + 54097);
    update_table_multi_page(3, 4, test_address);
    update_table_address(1, 4, page+3, test_address + 5 * PAGE_SIZE);

    int store = 2094;
    memcpy(&mem_block[FIRST_USER_PAGE + (page + 3) * PAGE_SIZE], &store, sizeof(int));

    int temp1;
    int temp2;

    printf("\n-----Original Setup-----\n");
    memcpy(&temp1, &mem_block[FIRST_USER_PAGE + (page + 3) * PAGE_SIZE], sizeof(int));
    memcpy(&temp2, &mem_block[FIRST_USER_PAGE + (page + 5) * PAGE_SIZE], sizeof(int));
    printf("IN ACTIVE PAGE: %d\n", temp1);
    printf("IN SWAP PAGE: %d\n", temp2);
    printf("ACTIVE TID: %d\n", get_active_tid(page+3));

    printf("\n-----Swapping Here-----\n");
    swap_pages(page+3, 4, page+5);

    memory_protect_page(page+3);

    memcpy(&temp1, &mem_block[FIRST_USER_PAGE + (page + 3) * PAGE_SIZE], sizeof(int));
    memcpy(&temp2, &mem_block[FIRST_USER_PAGE + (page + 5) * PAGE_SIZE], sizeof(int));
    printf("IN ACTIVE PAGE: %d\n", temp1);
    printf("IN SWAP PAGE: %d\n", temp2);
    printf("ACTIVE TID: %d\n", get_active_tid(page+3));

    printf("\n-----Swapping Here-----\n");
    swap_pages(page+3, 3, page+5);

    memcpy(&temp1, &mem_block[FIRST_USER_PAGE + (page + 3) * PAGE_SIZE], sizeof(int));
    memcpy(&temp2, &mem_block[FIRST_USER_PAGE + (page + 5) * PAGE_SIZE], sizeof(int));
    printf("IN ACTIVE PAGE: %d\n", temp1);
    printf("IN SWAP PAGE: %d\n", temp2);
    printf("ACTIVE TID: %d\n", get_active_tid(page+3));
}
*/

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
