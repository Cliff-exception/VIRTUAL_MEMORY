#include "myalloc.h"
#include "my_pthread_t.h"   // Order matters, don't switch myalloc with pthread!

void * test1() {

	int * x = (int *) myallocate(20,NULL, 0, 1); 

	int i = 0; 

	printf("Thread 1 ******************\n");

	while ( i <= 20) {
		x[i] = i; 
		i++; 
	}

	i = 0; 

	while (i <= 20) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}

	return NULL; 
	/*int * x = (int*) myallocate(20, NULL, 0, 1);
	x[0] = 2;
	x[1] = 3;

	pthread_yield();

	printf("%d:%d\n", x[0], x[1]);

	return NULL; */
}

void * test2() {
	int * x = (int *) myallocate(20,NULL, 0, 2); 

	int i = 0; 
	printf("Thread 2 ******************\n");
	while ( i <= 20) {
		x[i] = i; 
		i++; 
	}

	i = 0; 

	while (i <= 20) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
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

	int * x = (int *) myallocate(20,NULL, 0, 3); 

	int i = 0; 

	printf("Thread 3 ******************\n");

	while ( i <= 20) {
		x[i] = i; 
		i++; 
	}

	i = 0; 

	while (i <= 20) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
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

	int * x = (int *) myallocate(20,NULL, 0, 4); 

	int i = 0; 

	printf("Thread 4 ******************\n");

	while ( i <= 20) {
		x[i] = i; 
		i++; 
	}

	i = 0; 

	while (i <= 20) {

		printf("X[%d] = %d \n", i, x[i] );
		i++; 
	}

	return NULL; 
}

int main() {
	my_pthread_t tid_1;
	my_pthread_t tid_2;
	my_pthread_t tid_3;
	my_pthread_t tid_4; 

	pthread_create(&tid_1, NULL, &test1, NULL);
	pthread_create(&tid_2, NULL, &test2, NULL);
	pthread_create(&tid_3, NULL, &test3, NULL);
	pthread_create(&tid_4, NULL, &test4, NULL); 

	pthread_join(tid_1, NULL);
	pthread_join(tid_2, NULL);
	pthread_join(tid_3, NULL);
	pthread_join(tid_4,NULL); 

	return 0;
}