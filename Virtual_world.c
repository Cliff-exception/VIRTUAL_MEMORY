
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

	/*We want the next IO operation to start at the begining of the file
	  So we lseek to the begining of the file to initialize the swap file
	*/

	if ( lseek(swap_file_descriptor, 0 , SEEK_SET) == -1 ) {

		printf("error setting up swipe file\n");
		exit(EXIT_FAILURE); 		
	}

	/* now set the offset of the swapfile to the size of our memory space by using lseek
	   lseek will increase the size of our swafile by the memsize
	*/

	int newOffset = lseek(swap_file_descriptor, MEM_SIZE, SEEK_SET); 

	printf("this is the new size: %d\n", newOffset );

	return 1; 

}

int main(int argc, char const *argv[])
{
	swap_space_init(); 
	return 0;
}


