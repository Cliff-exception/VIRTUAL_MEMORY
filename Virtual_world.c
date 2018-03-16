
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
// define file descriptor for swapfile

int swap_file_descriptor; 


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

    // and we are done with setting up the swapfile, 
    // the swapfile grows each time we write to it so there is not need 
    // initialize size



/*
    for testing purposes
    below is a test progran to show how lseek works
*/ 

/*
    char * c = "Hello"; 

    write(swap_file_descriptor, c, strlen(c)); 

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

/*

to_mem_offset: offset of the page (into the page_table) of the page being written to memory from page table
out_mem_offset: offset of the page (into swap_file) of the page being written out of memory into page table

*/

int get_from_memory ( int to_mem_offset, int out_mem_offset  ) {

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
    void * to_mem = mem_block[(KERNEL_MEMORY + to_mem_offset)*PAGE_SIZE]; 

    // unprotect the page being written into memory (swap_file)
    if ( mprotect(to_mem, PAGE_SIZE, PROT_READ | PROT_WRITE)  == -1) {

        printf("Error unprotecting memory\n");
        exit(EXIT_FAILURE); 
    }


    /* now write the page into swap_file
       note: we are doing a direct swapping, the page going out of the page table
       will take the spot (in the swapfile) of the page coming into the page table
       vise versa  
    */

    int seek_size_2 = lseek(swap_file_descriptor, out_mem_offset*PAGE_SIZE, SEEK_SET); 

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

int main(int argc, char const *argv[])
{
    swap_space_init(); 
    return 0;
}


