#include <stdio.h>
//#include </home/ahmed/glibc-2.23/sysdeps/nptl/pthread.h>
#include<pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#define NTHREADS 10

pthread_barrier_t bar;

/***********************Simulating******************************************
parent ***----------*--------********
thread1  ***********
thread2  *---------**********		    
/***********************************************************************/


void * thread_func1(void * args){
	long long int k = 0;	
	for(int i =0; i< 50000 ; i++){
		for(int j = 0 ; j < 10000; j++){
			 k = i*j;
		}
	}
	int self_pid =(int) syscall(__NR_gettid);
	pthread_barrier_wait(&bar);
         printf("%d\tv_time returned %lf\n",self_pid,(double) syscall(333,self_pid));

}

void * thread_func2(void * args){
	long long int k = 0;	
	for(int i =0; i< 50 ; i++){
		k = i;
	}
	int self_pid =(int) syscall(__NR_gettid);
	pthread_barrier_wait(&bar);
         printf("%d\tv_time returned %lf\n",self_pid,(double) syscall(333,self_pid));

}



int main()
{
  
	pthread_t threads[NTHREADS];
	pthread_barrier_init(&bar,NULL,2);
	pthread_create(&threads[0], NULL, thread_func1, "...");

	pthread_create(&threads[1], NULL, thread_func2, "...");
	//pthread_create(&threads[2], NULL, thread_func, "...");
	pthread_join(threads[0], NULL);

	pthread_join(threads[1], NULL);
       // pthread_join(threads[2], NULL);
	int self_pid =(int) syscall(__NR_gettid);

	 printf("%d\tgroup v_time returned %lf\n",self_pid,(double) syscall(335,self_pid));

	return 0;

}
