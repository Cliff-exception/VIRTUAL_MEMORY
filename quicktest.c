	#include <stdio.h>
	#include <stdlib.h> 
	#include <time.h>


	int main () {

	
		 srand(time(NULL)); 

		int n = rand() % 200;  

		printf("number: %d\n" , n); 

		return 0; 
	}
