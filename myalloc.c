#include "myalloc.h"
#include "my_pthread_t.h"

static void handler(int sig, siginfo_t *si, void *unused) {
    //if (si->si_addr > &mem_block && si->si_addr < &mem_block + MEM_SIZE) {
        printf("Got SIGSEGV at address: 0x%lx\n",(long) si->si_addr);
        int page = get_page_number_real_phy((unsigned long) si->si_addr);
        printf("sigsegv page: %d\n", page);
    //    printf("TID: %d\n", get_curr_tid());
        int tid = get_curr_tid();
    //    printf("Sigsegv calling swap!\n");
        int table_entry = get_table_entry(tid, page);

        if (table_entry < 0)
            swap_pages(page, tid, get_unused_page());
        else
            swap_pages(page, tid, get_page_from_table(tid, page));
    //}
    /*
    else {
        printf("Real segfault!\n");
        exit(1);
    }
    */
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

void print_blk_list(block_meta * blk){

    if(!blk) return;
    printf("blk size: %d\n", blk->blk_size);
    printf("free size: %d\n", blk->free_size);
    
    print_blk_list(blk->next);
}

void print_blk_meta(block_meta * blk){
//printf("one blk!\n");
    if(!blk) return;
    printf("blk size: %d\n", blk->blk_size);
    printf("free size: %d\n", blk->free_size);
  
}

block_meta * init_block_meta_page(int tid_req, int x, block_meta * prev, block_meta * next) {
    //size_t block_size = prev->free_size - x - sizeof(block_meta);
    size_t block_size = x + sizeof(block_meta);

    block_meta temp_block;
    temp_block.p_type = UNASSIGNED;
    temp_block.blk_size = block_size;
    //temp_block.free_size = block_size;
    temp_block.tid = tid_req;
    temp_block.prev = prev;
    temp_block.next = next;
    
    
    block_meta * address = (block_meta *) ((unsigned long)prev + x + sizeof(block_meta));
    address = (block_meta *)safely_align_block((unsigned long)address);
    if(((unsigned long)address - (unsigned long)(prev) + sizeof(block_meta)) >= prev->free_size){
    	prev->free_size = 0;
    	return prev;
    }

    temp_block.free_size = (unsigned long)next - (unsigned long)address;

    int page = get_page_number_real_phy((unsigned long)address);
    
    if(get_table_entry(tid_req,page)<0)
    	swap_pages(page,tid_req,get_unused_page());
    else
    	swap_pages(page,tid_req,get_page_from_table(tid_req,page));

    memcpy((void*)address, &temp_block, sizeof(block_meta));

    if (next != NULL)
        next->prev = address;
    prev->next = address;

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

    swap_pages(0, tid_req, get_unused_page());
    memcpy((void*)address, &temp_block, sizeof(block_meta));

    return address;
}
void align_pages(int page1, int page2, int tid_req){
	int i = page1;
        int unused_page;
        while(i < page2){
            unused_page= get_unused_page();
            swap_pages(i, tid_req, unused_page);
            i++;
        }
}

block_meta * find_block(int tid_req, size_t x) {
    // TODO: Make sure that if found block has an empty page, note empty page as used.

    block_meta * b_meta = (block_meta *) &mem_block[FIRST_USER_PAGE];
    block_meta * next_meta;
    int max_page;

    int current_loc_page_zero = get_table_entry(tid_req, 0);
    if (current_loc_page_zero == OUT_OF_BOUNDS) {
        next_meta = (block_meta *) &mem_block[FIRST_USER_PAGE + sizeof(block_meta) + x];
        max_page = ((unsigned long)(next_meta) + sizeof(block_meta) - (unsigned long)&mem_block[FIRST_USER_PAGE]) / PAGE_SIZE + 1;
    
        align_pages(0, max_page, tid_req);
        b_meta = init_block_meta_page_zero(tid_req);
        
        b_meta->next = next_meta;
        next_meta->prev = b_meta;
        next_meta->next = NULL;
        
        
        next_meta->free_size = next_meta->prev->free_size - x - sizeof(block_meta); //maybe the source of 
        //printf("prev free size: %d, free size: %d\n",next_meta->prev->free_size, next_meta->free_size);
        b_meta->free_size = x;
        
        next_meta->blk_size = x;
        b_meta->blk_size = 0;
            
        b_meta->p_type = DATA;
        next_meta->p_type = UNASSIGNED;
    
        b_meta->tid = tid_req;
        next_meta->tid = tid_req; 
        
        //print_blk_meta(next_meta);//
        return next_meta;
    }

    int in_page = get_page_number_real_phy((unsigned long) b_meta);
    swap_pages(in_page, tid_req, get_page_from_table(tid_req, in_page));

    // Need to swap pages into place to walk linked list.
    while (b_meta->free_size < (sizeof(block_meta) + x)) {
    	//printf("curr size: %d\n",b_meta->free_size);

        if (b_meta->next == NULL)
            return NULL;

        b_meta = b_meta->next;

        in_page = get_page_number_real_phy((unsigned long) b_meta);
        swap_pages(in_page, tid_req, get_page_from_table(tid_req, in_page));
    }
   
    block_meta * new_meta = init_block_meta_page(tid_req, x, b_meta, b_meta->next);
    new_meta->blk_size = x;
    new_meta->free_size = x;
    
    //print_blk_meta(new_meta);//
    
    return new_meta;
}

void protect_all_tid_pages(int tid){
    int curr_page = 0, prot_page;
    while(curr_page < NUM_USER_PAGES){
        if (is_in_memory(tid, curr_page)) {
            prot_page = get_page_from_table(tid, curr_page);
            memory_protect_page(prot_page);
        }
        curr_page++;
    }
}

void unprotect_all_tid_pages(int tid){
    int curr_page = 0, prot_page;
    while(curr_page < NUM_USER_PAGES){
        if (is_in_memory(tid, curr_page)) {
            prot_page = get_table_entry(tid, curr_page);
            memory_unprotect_page(prot_page);
        }
        curr_page++;
    }
}

void swap_protection(int out_tid,int in_tid){
    
    protect_all_tid_pages(out_tid);

    unprotect_all_tid_pages(in_tid);
}

//void * myallocate(size_t x, __FILE__, __LINE__, THREADREQ){
void * myallocate(size_t x, char * file, int linenum, int tid_req){
    
    assert(x>0);
    //if(tid_req == LIBRARYREQ ){return NULL;} 
        // maybe later
    if(tid_req > NUM_PROCESSES ||
       tid_req < 0 ) {
    
        return NULL;
    }
    
    block_meta * first_fit = find_block(tid_req, x);
    if(!first_fit) {
        printf("physical memory full!\n");
    
        return NULL;
    }
    
    int i = 0;
    int page = get_page_number_real_phy((unsigned long)first_fit);
    int f = (x + 2 *sizeof(block_meta))/ PAGE_SIZE + 1;
    while(i < f){
 	    update_table_entry(tid_req,page+i,page+i,0);
        note_page_used(page+i);
    	i++;
    }
    
   
    //print_blk_meta(first_fit->prev);
    return (void*) (first_fit->prev + 1);
}



//mydeallocate(x, __FILE__, __LINE__, THREADREQ)
void mydeallocate(void * ptr, char * file, int linenum, int tid_req){
	block_meta * target = (block_meta *)ptr;
	
	
	target--;
	//print_blk_meta(target);
	target = target->next;
	//printf("||||||||||||||||||||||\n");
	//print_blk_meta(target);
	
	//printf("%d\n",target);

    target->p_type = UNASSIGNED;
	
	block_meta* prev = NULL, *next = NULL;
    if (target->prev) {
        prev = target->prev;

        if (prev->p_type == UNASSIGNED) { //coalescing with prev blk
            prev->prev->next = target;
            prev->prev->free_size = prev->blk_size + 
                                    target->blk_size + 
                                    sizeof(block_meta);

            target->prev = prev->prev;
        }
    }

    if (target->next) {
        next = target->next;
        if (next->p_type == UNASSIGNED) { // coalescing with next blk
            next->prev = target->prev;
            target->prev->next = next;
            target->prev->free_size = target->free_size +
                                      prev->free_size +
                                      sizeof(block_meta);
        }
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
    int location;
    int current_page;
    for ( ; i < NUM_PROCESSES; i++) {
        location = get_location(i, page);
        current_page = get_table_entry(i, page);
        //printf("%d:%d\n", page, current_page);
        if (location == 0 && current_page == page)
            return i;
    }

    return -1;
}

void swap_pages(int in_pos_page, int out_tid, int out_pos_page) {
    //printf("%d : %d : %d\n", in_pos_page, out_tid, out_pos_page);
    //printf("IN_POS_PAGE : %d\n", in_pos_page);
    //printf("TID         : %d\n", out_tid);
    //printf("OUT_POS_PAGE: %d\n", out_pos_page);

    memory_unprotect_page(in_pos_page);
    int in_offset = FIRST_USER_PAGE + in_pos_page * PAGE_SIZE;
    int out_offset = FIRST_USER_PAGE + out_pos_page * PAGE_SIZE;
    
    //printf("IN_POS_PAGE : %d\n", in_offset);
    //printf("OUT_POS_PAGE: %d\n", out_offset);
	
    // Swap pages within memory utilizing swap page location as pivot.
    memcpy(&mem_block[SWAP_PAGE], &mem_block[in_offset], PAGE_SIZE);
    memcpy(&mem_block[in_offset], &mem_block[out_offset], PAGE_SIZE);
    memcpy(&mem_block[out_offset], &mem_block[SWAP_PAGE], PAGE_SIZE);

    memory_protect_page(out_pos_page);
    memory_unprotect_page(in_pos_page);
    int in_tid = get_active_tid(in_pos_page);

    if (in_tid > -1)
        update_table_entry(in_tid, in_pos_page, out_pos_page, 0);
    update_table_entry(out_tid, in_pos_page, in_pos_page, 0);

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

// location: 0 - memory   1 - upper file locations
int create_table_entry(int location, int page) {
    return (location << 14) + page;
}

int get_location(int tid, int page) {
    int table_entry = get_table_entry(tid, page);
    return table_entry >> 14;
}

int get_page_from_table(int tid, int page) {
    int table_entry = get_table_entry(tid, page);
    return table_entry & 0xFFF;
}

int is_in_memory(int tid, int page) {
    int table_entry = get_table_entry(tid, page);

    if (get_location(tid, page) == 0)
        return 1;
    else
        return 0;
}

int is_in_upper_swap(int tid, int page) {
    int table_entry = get_table_entry(tid, page);

    if (get_location(tid, page) == 1)
        return 1;
    else
        return 0;
}

int is_in_lower_swap(int tid, int page) {
    int table_entry = get_table_entry(tid, page);

    if (get_location(tid, page) == 2)
        return 1;
    else
        return 0;
}

//int main() {
//    int entry = create_table_entry(0, 12);
//    printf("Entry: %d\n", entry);
//}

// Update the page that is stored for a given tid inside the inverted page
// table.
void update_table_entry(int tid, int page, int new_page, int location) {
    int offset = get_table_offset(tid, page);
    int table_entry = create_table_entry(location, new_page);
//    int offset = tid * NUM_USER_PAGES + (page * sizeof(int));

//    int has_block_meta  = contains_block_meta(tid, page);

//    if (has_block_meta > 0)
//        new_page = (1 << 12) + new_page;
    
    //if( get_table_entry(tid,page) < 0 ){
    note_page_used(page);
    //}
    
    //memcpy(&mem_block[offset], &new_page, sizeof(int));
    memcpy(&mem_block[offset], &table_entry, sizeof(int));

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

    int table_entry = get_table_entry(tid, page);

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
        if (page_used == 0) {
            //printf("Page free: %d\n", i);
            note_page_used(i);
            return i;
        }
    }

    return -1;
}

//------------------------------------------------------------------------------
//
// Functions that allow keeping track of which pages are currently being used
// or free.
//
//------------------------------------------------------------------------------

// portion: 0 - upper part of swap file, 1 - lower part of swap file
int get_note_page_offset_file(int portion, int page) {
    return get_table_offset((NUM_PROCESSES - 2) + portion, page);
}

void note_page_used_file(int page) {
    int portion = 0;
    if (page > NUM_USER_PAGES) {
        portion = 1;
        page = page - NUM_USER_PAGES;
    }

    int offset = get_note_page_offset_file(portion, page);
    int used = 1;
    memcpy(&mem_block[offset], &used, sizeof(int));
}

void note_page_unused_file(int page) {
    int portion = 0;
    if (page > NUM_USER_PAGES) {
        portion = 1;
        page = page - NUM_USER_PAGES;
    }

    int offset = get_note_page_offset_file(portion, page);
    int used = 0;
    memcpy(&mem_block[offset], &used, sizeof(int));
}

int get_unused_page_file() {
    int i = 0;
    int page_used;
    for ( ; i < NUM_USER_PAGES; i++) {
        memcpy(&page_used, &mem_block[get_note_page_offset_file(0, i)], 
            sizeof(int));
        if (page_used == 0) {
            //printf("Page free: %d\n", i);
            note_page_used(i);
            return i;
        }
    }

    for ( ; i < NUM_USER_PAGES; i++) {
        memcpy(&page_used, &mem_block[get_note_page_offset_file(1, i)], 
            sizeof(int));
        if (page_used == 0) {
            //printf("Page free: %d\n", i);
            note_page_used(i + NUM_USER_PAGES);
            return (i + NUM_USER_PAGES);
        }
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
    return physical_address / PAGE_SIZE;
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



/*---------------------------------------------------- 


// functions for the swap_file

------------------------------------------------------*/

int swap_space_init() {
    swap_file_descriptor = open("swap_file", O_CREAT | O_RDWR | S_IWUSR | S_IRUSR); 

    if ( swap_file_descriptor == -1 ) {

        printf("error opening swapfile\n");
        exit(EXIT_FAILURE); 
    }


    if ( lseek(swap_file_descriptor, 0 , SEEK_SET) == -1 ) {

        printf("error setting up swipe file\n");
        exit(EXIT_FAILURE);         
    }

    // initialize the memory space
    char buffer [8000000]; 
    int write1 = write(swap_file_descriptor,buffer, 8000000);
    int write2 = write(swap_file_descriptor,buffer, 8000000);  

    //printf("First write: %d\n", write1);
    //printf("Second write: %d\n", write2);


    lseek(swap_file_descriptor,0,SEEK_SET);

    return 1; 

}

void evict_page ( unsigned long address, int swap_file_offset ) {

    int seek_size = lseek(swap_file_descriptor, swap_file_offset*PAGE_SIZE, SEEK_SET); 

    if ( seek_size == -1 ) {

        printf("Error finding swap position for evcited page\n");
        exit(EXIT_FAILURE); 
    }

    int bytes_written = write(swap_file_descriptor, (void*)address, PAGE_SIZE); 

    if ( bytes_written <= 0 ) {

        printf("Error evicting page into swap_file\n");
        exit(EXIT_FAILURE); 
    }

    return; 
}

/*

to_mem_offset: offset of the page (into the page_table) of the page being written to memory from page table
out_mem_offset: offset of the page (into swap_file) of the page being written out of memory into page table
to_mem_swap_offset: a pointer to the offset (in the swapfile) of the page being moved into the swapfile
                    the pointer allows us to assign a new offset if the page does yet reside in the swapfile
*/

int get_from_memory ( unsigned long to_mem_offset, int out_swap_offset) {

    // buffer for the page we are reading from swapfile
    char from_mem[PAGE_SIZE]; 

    int seek_size = lseek(swap_file_descriptor, out_swap_offset*PAGE_SIZE, SEEK_SET); 

    if ( seek_size == -1 ) {

        printf("Error finding page finding memory\n");
        exit(EXIT_FAILURE); 
    }

    int read_bytes = read(swap_file_descriptor, from_mem, PAGE_SIZE);

    if ( read_bytes == 0  ) {

        printf("Error reading from swap_file\n");
        exit(EXIT_FAILURE); 
    } 

    // unprotect the page being written into memory (swap_file)
    if ( mprotect((void*)to_mem_offset, PAGE_SIZE, PROT_READ | PROT_WRITE)  == -1) {

        printf("Error unprotecting memory\n");
        exit(EXIT_FAILURE); 
    }


    /* now lseek() to the position (in the swap_file) of the page being place in the swapfile
       and write it to the swapfile
    */

    int seek_size_2 = lseek(swap_file_descriptor, out_swap_offset*PAGE_SIZE, SEEK_SET); 

    if ( seek_size_2 == -1 ) {

        printf("Error getting offset of page in memory\n");
        exit(EXIT_FAILURE); 
    }

    int bytes_written = write(swap_file_descriptor, (void*)to_mem_offset, PAGE_SIZE); 

    if ( bytes_written <= 0 ){

        printf("error writing page to memory\n");
        exit(EXIT_FAILURE); 
    }


    // now we shall do a direct swap using memcpy
    memcpy((void*)to_mem_offset, from_mem, PAGE_SIZE ); 

    // might need to modify metadata of pages

    return 1; 

}

