#include "myalloc.h"
#include "my_pthread_t.h"   // Order matters, don't switch myalloc with pthread!


int called = 0;
void * str_test1 () {

	char * x = (char*)malloc(sizeof(char)*20000); 

	my_pthread_yield(); 

	x[19993] = 'h'; 
	x[19994] = 'e'; 
	x[19995] = 'l'; 
	x[19996] = 'l'; 
	x[19997] = 'o'; 

	int i = 19993; 

	while (i <= 19997) {

		printf("%c\n", x[i] );
		i++; 
	}

	printf("\n");
}


void * str_test2 () {

	char * x = (char*)malloc(sizeof(char)*20000); 

	my_pthread_yield(); 

	x[19993] = 'c'; 
	x[19994] = 'l'; 
	x[19995] = 'i'; 
	x[19996] = 'f'; 
	x[19997] = 'f';
	x[19998] = 'o'; 
	x[19999] = 'r'; 
	x[20000] = 'd'; 

	int i = 19993; 

	while (i <= 20000) {

		printf("%c\n", x[i] );
		i++; 
	}


	//printf("\n");
}



void * test1( void * args) {


	char * hate;
	if(called < 10){
		hate = (char *)shalloc(32);
		printf("hate: %x\n",hate);
		*hate;
		strcpy(hate, "FTW");
		//*hate = 3;
	
		
		printf("%s\n",hate);
	}
	

	int * num = (int*) args; 
	int mult = *num; 

	int * x = (int *) malloc(15625 * sizeof(int)); 

	int i = 0; 

	printf("Thread %d First Array ******************\n", mult);

	while ( i <= 10) {
		x[i] = i * mult; 
		i++; 
	}

	i = 0; 

	while (i <= 10) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}

//	my_pthread_yield(); 

	printf("Thread %d second array ************\n", mult);
	int * z = (int*)malloc(20*sizeof(int)); 
//	my_pthread_yield();

	int j = 0; 

	while ( j <= 10 ) {

		z[j] = i * mult; 
		i++; 
		j++;
	}

	j = 0; 

	while ( j <= 10) {

		printf("Z[%d] = %d \n", j , z[j] );
		j++; 
	}


	char * y = (char*)malloc(sizeof(char)*20000); 

//	my_pthread_yield(); 

	//printf("String of thread_1\n");

/*	y[19993] = 'h'; 
	y[19994] = 'e'; 
	y[19995] = 'l'; 
	y[19996] = 'l'; 
	y[19997] = 'o'; 

	int k = 19993; 

	while (k <= 19997) {

		printf("%c\n", y[k] );
		k++; 
	}

	printf("\n");	 */


	return NULL; 
	/*int * x = (int*) myallocate(20, NULL, 0, 1);
	x[0] = 2;
	x[1] = 3;

	pthread_yield();

	printf("%d:%d\n", x[0], x[1]);

	return NULL; */
}

void * test2() {
	int * x = (int *) malloc(15625 * sizeof(int)); 

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
	int * z = (int*)malloc(15625*sizeof(int)); 
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

	return NULL; 
}

int main() {

	my_pthread_t tid_1 = 1;
/*	my_pthread_t tid_2;
	my_pthread_t tid_3;
	my_pthread_t tid_4; 
	my_pthread_t tid_5; 
	my_pthread_t tid_6;  */
/*
	pthread_create(&tid_1, NULL, &test1, NULL);
	pthread_create(&tid_2, NULL, &test2, NULL);
	pthread_create(&tid_3, NULL, &test3, NULL);
	pthread_create(&tid_4, NULL, &test4, NULL); */ 
	//pthread_create(&tid_5, NULL, &str_test1, NULL);
	//pthread_create(&tid_6, NULL, &str_test2, NULL); 

	while ( tid_1 < 64 ) {
//	while (tid_1 < 2) {
		pthread_create(&tid_1, NULL, &test1, &tid_1); 
		tid_1++; 
	}

	tid_1 = 1; 

	while ( tid_1 < 64 ) {
//	while (tid_1 < 2) {
	pthread_join(tid_1, NULL);
	tid_1++; 
/*	pthread_join(tid_2, NULL);
	pthread_join(tid_3, NULL);
	pthread_join(tid_4,NULL); 
	pthread_join(tid_5,NULL); 
	pthread_join(tid_6, NULL); */

	}

	return 0;
}
