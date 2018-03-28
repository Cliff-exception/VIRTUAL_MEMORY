#include "myalloc.h"
#include "my_pthread_t.h"
char * s;
void * str_test1 () {

	char * x = (char*)shalloc(sizeof(char)*200); 
	s = x;

	strcpy(x,"hello shared");
	printf("%x\n",x);
	pthread_exit(x);
}


void * str_test2 (void * x) {

	char * str = (char *)x; 
	printf("%s\n",str);
	return NULL;
}


int main(){
	my_pthread_t tid_1;
	my_pthread_t tid_2;
	 
	char * str;
	pthread_create(&tid_1, NULL, &str_test1, NULL);
	
	pthread_join(tid_1,(void **)&str);
	printf("%x\n",str);
	
	pthread_create(&tid_2, NULL, &str_test2, str);
	
	pthread_join(tid_2,NULL);
	
	
	return 0;
}
