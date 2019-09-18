#include <stdio.h>
//#include </home/ahmed/glibc-2.23/sysdeps/nptl/pthread.h>
#include<pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#define NTHREADS 10
pthread_mutex_t lock;


/***********************Simulating******************************************
parent ***----------*--------********
thread1  ***********
thread2  *---------**********		    
/***********************************************************************/


void * thread_func(void * args){
	long long int k = 0;	
	pthread_mutex_lock(&lock);
	for(int i =0; i< 5000 ; i++){
		for(int j = 0 ; j < 10000; j++){
			 k = i*j;
		}
	}
	int self_pid =(int) syscall(__NR_gettid);
         printf("%d\tv_time returned %lf\n",self_pid,(double) syscall(336,self_pid));

	pthread_mutex_unlock(&lock);
}


int main()
{
  
	pthread_t threads[NTHREADS];
	pthread_mutex_init(&lock, NULL);

//	printf("%d\t%d\n",lock.__data.lock_acquisition_g_time,sizeof(lock));	
	pthread_create(&threads[0], NULL, thread_func, "...");

	pthread_join(threads[0], NULL);


	return 0;

}
