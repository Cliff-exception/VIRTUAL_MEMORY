
#include "myalloc.c"

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stddef.h>
#include<sys/mman.h>
#include<unistd.h>
#include<signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define taken 4
// define file descriptor for swapfile

int swap_file_descriptor; 

int swap_offset = 0; 


int swap_space_init() {

    /* set up function that handles page_faults
       The "page_fault" function has to be defined later

    */
    struct sigaction sa; 
    sa.sa_flags = SA_SIGINFO; 
    sigemptyset(&sa.sa_mask); 
    //sa.sa_sigaction = page_fault; 

    if ( sigaction(SIGSEGV, &sa, NULL) == -1 ) {

        printf("hmmm...error setting up the signal handler\n");
        exit(EXIT_FAILURE); 
    }

    /*
        Creating the swapfile, a littl overkill with the permissions but better safe than sorry 
    */
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

    printf("First write: %d\n", write1);
    printf("Second write: %d\n", write2);


    //lseek(swap_file_descriptor,0,SEEK_SET); 

    int bytes = lseek(swap_file_descriptor, 0, SEEK_END); 

    printf("size of swap_file : %d\n", bytes );



    // and we are done with setting up the swapfile, 
    // the swapfile grows each time we write to it so there is not need 
    // initialize size



/*
    for testing purposes
    below is a test progran to show how lseek works
*/ 
 /*
    lseek(swap_file_descriptor,0,SEEK_SET); 
    char * c = "Hello"; 

    write(swap_filele_descriptor, c, strlen(c)); 

    lseek(swap_file_descriptor, 0, SEEK_SET);

    char buff[6]; 

    read(swap_file_descriptor, buff, 5); 
    buff[6] = '\0'; 

     printf("%s\n", buff );

     bzero(buff, strlen(buff)); 

    lseek(swap_file_descriptor, 6, SEEK_SET); 

    char * d = "howdy"; 

    write(swap_file_descriptor,d,strlen(d)); 

    lseek(swap_file_descriptor,6,SEEK_SET); 

    read(swap_file_descriptor,buff,5); 
    buff[6] = '\0'; 

    printf("%s\n", buff );

                            */


    return 1; 

}


// given a 
int store_in_memory 

/*

to_mem_offset: offset of the page (into the page_table) of the page being written to memory from page table
out_mem_offset: offset of the page (into swap_file) of the page being written out of memory into page table
to_mem_swap_offset: a pointer to the offset (in the swapfile) of the page being moved into the swapfile
                    the pointer allows us to assign a new offset if the page does yet reside in the swapfile
*/

int get_from_memory ( int to_mem_offset, int out_mem_offset) {

    // buffer for the page we are reading from swapfile
    char from_mem[PAGE_SIZE]; 

    int seek_size = lseek(swap_file_descriptor, out_mem_offset*PAGE_SIZE, SEEK_SET); 

    if ( seek_size == -1 ) {

        printf("Error finding page finding memory\n");
        exit(EXIT_FAILURE); 
    }

    int read_bytes = read(swap_file_descriptor, from_mem, PAGE_SIZE);

    if ( read_bytes == 0  ) {

        printf("Error reading from swap_file\n");
        exit(EXIT_FAILURE); 
    } 

    // grab page being written to memory from page table
    char * to_mem = &mem_block[(KERNEL_MEMORY + to_mem_offset)*PAGE_SIZE]; 

    // unprotect the page being written into memory (swap_file)
    if ( mprotect(to_mem, PAGE_SIZE, PROT_READ | PROT_WRITE)  == -1) {

        printf("Error unprotecting memory\n");
        exit(EXIT_FAILURE); 
    }
 
    int swap_file_offset  = *to_mem_swap_offset; 

    /* if the page being swapped out does not currently live in the swapfile
       assign it the next free index on the swapfile and write it to that index
    */
    if ( swap_file_offset == -1 ) {

        swap_file_offset = swap_offset++; 
        *to_mem_swap_offset = swap_file_offset; 
    }


    /* now lseek() to the position (in the swap_file) of the page being place in the swapfile
       and write it to the swapfile
    */
    int seek_size_2 = lseek(swap_file_descriptor, swap_file_offset*PAGE_SIZE, SEEK_SET); 

    if ( seek_size_2 == -1 ) {

        printf("Error getting offset of page in memory\n");
        exit(EXIT_FAILURE); 
    }

    int bytes_written = write(swap_file_descriptor, to_mem, PAGE_SIZE); 

    if ( bytes_written <= 0 ){

        printf("error writing page to memory\n");
        exit(EXIT_FAILURE); 
    }


    // now we shall do a direct swap using memcpy
    memcpy(to_mem, from_mem, PAGE_SIZE ); 

    // might need to modify metadata of pages

    return 1; 

}

void swap_page_int_table () {

    
}




void protect_pages ( unsigned int in, unsigned int out ) {



}

int main(int argc, char const *argv[])
{
    swap_space_init(); 
    return 0;
}


