#include "myalloc.h"
#include "my_pthread_t.h"   // Order matters, don't switch myalloc with pthread!



/*void * str_test () {

	char * x = (char*)malloc(sizeof(char)*6000); 

}
*/
void * test1() {

	int * x = (int *) malloc(20 * sizeof(int)); 

	int i = 0; 

	printf("Thread 1 First Array ******************\n");

	while ( i <= 10) {
		x[i] = i; 
		i++; 
	}

	i = 0; 

	while (i <= 10) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}
free(x);
	my_pthread_yield(); 

	printf("Thread 1 second array ************\n");
	int * z = (int*)malloc(20*sizeof(int)); 
	//my_pthread_yield();
	
	int j = 0; 

	while ( j <= 10 ) {

		z[j] = i; 
		i++; 
		j++;
	}

	j = 0; 

	while ( j <= 10) {

		printf("Z[%d] = %d \n", j , z[j] );
		j++; 
	}
free(z);


int * a =  (int *) malloc(20);
int * b =  (int *) malloc(20);
int * c =  (int *) malloc(20);
int * d =  (int *) malloc(20);

free(a);free(b);free(c);free(d);


	//return NULL; 
	/*
	int * x = (int*) myallocate(20, NULL, 0, 1);
	x[0] = 2;
	x[1] = 3;

	pthread_yield();

	printf("%d:%d\n", x[0], x[1]);
	*/

	return NULL; 
}

void * test2() {
	int * x = (int *) malloc(20 * sizeof(int)); 

	int i = 0; 
	printf("Thread 2 first array ******************\n");
	while ( i <= 10) {
		x[i] = i*2; 
		i++; 
	}

	i = 0; 

	while (i <= 10) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}

	my_pthread_yield(); 
	printf("Thread 2 second array ************\n");
	
free(x);
	int * z = (int*)malloc(20*sizeof(int)); 
	//my_pthread_yield();

	int j = 0; 

	while ( j <= 10 ) {

		z[j] = i*2; 
		i++; 
		j++;
	}

	j = 0; 

	while ( j <= 10) {

		printf("Z[%d] = %d \n", j , z[j] );
		j++; 
	}
free(z);
	return NULL; 
	/*int * x = (int*) myallocate(20, NULL, 0, 2);
	x[0] = 4;
	x[1] = 5;

	pthread_yield();

	printf("%d:%d\n", x[0], x[1]);

	return NULL;*/
}

void * test3() {

	int * x = (int *) malloc(20 * sizeof(int)); 

	int i = 0; 

	printf("Thread 3 first array *******************\n");

	while ( i <= 10) {
		x[i] = i*3; 
		i++; 
	}

	i = 0; 

	while (i <= 10) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}

	my_pthread_yield(); 
free(x);
	printf("Thread 3 second array ************\n");
	int * z = (int*)malloc(20*sizeof(int)); 
	//my_pthread_yield();

	int j = 0; 

	while ( j <= 10 ) {

		z[j] = i*3; 
		i++; 
		j++;
	}

	j = 0; 

	while ( j <= 10) {

		printf("Z[%d] = %d \n", j , z[j] );
		j++; 
	}
free(z);
	return NULL; 
/*	int * x = (int*) myallocate(20, NULL, 0, 3);
	x[0] = 6;
	x[1] = 7;

	pthread_yield();

	printf("%d:%d\n", x[0], x[1]);

	return NULL; */
}

void * test4 () {

	int * x = (int *) malloc(20 * sizeof(int)); 

	int i = 0; 

	printf("Thread 4 ******************\n");

	while ( i <= 10) {
		x[i] = i*4; 
		i++; 
	}

	i = 0; 
free(x);
	while (i <= 10) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}

	my_pthread_yield(); 

	printf("Thread 4 second array ************\n");
	int * z = (int*)malloc(20*sizeof(int)); 
	//my_pthread_yield();

	int j = 0; 

	while ( j <= 10 ) {

		z[j] = i*4; 
		i++; 
		j++;
	}

	j = 0; 

	while ( j <= 10) {

		printf("Z[%d] = %d \n", j , z[j] );
		j++; 
	}	
free(z);
/*
	char * c = (char *) malloc(20);
	strcpy(c,"FTW!");
	printf("::::::::::::::::::::::::::\n");
	printf("\n\n\n\n\n\n\n\n\n\n\n\n%s\n\n\n\n\n\n\n\n",c);
*/	
	return NULL; 
}

int main() {
	my_pthread_t tid_1;
	my_pthread_t tid_2;
	my_pthread_t tid_3;
	my_pthread_t tid_4; 

	pthread_create(&tid_1, NULL, &test1, NULL);
	//pthread_create(&tid_2, NULL, &test2, NULL);
	//pthread_create(&tid_3, NULL, &test3, NULL);
	//pthread_create(&tid_4, NULL, &test4, NULL); 

	pthread_join(tid_1, NULL);
	//pthread_join(tid_2, NULL);
	//pthread_join(tid_3, NULL);
	//pthread_join(tid_4,NULL); 
	
	
	
	
	return 0;
}
