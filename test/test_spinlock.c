#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>



//pthread_spinlock_t spinlock;
int spinlock;
#define NTHREADS 10
pthread_mutex_t lock;


void * thread_func(void * args){
	
	for(int i =0; i< 5000 ; i++){
		pthread_spin_lock(&spinlock);
		printf("Hi its jus a test1\n");
//	
		sleep(1);
		float s = 0;
		for (int j =0; j < 10000; j++){
			float d = j*j;
			s +=d;
		}
		printf("Hi its jus a test2\n");
   		pthread_spin_unlock(&spinlock);

	}
}

//	
int main()
{
  
	pthread_t threads[NTHREADS];
	pthread_spin_init(&spinlock, 0);

//	printf("%d\t%d\n",lock.__data.lock_acquisition_g_time,sizeof(lock));	
	pthread_create(&threads[0], NULL, thread_func, "...");

	pthread_create(&threads[1], NULL, thread_func, "...");
	pthread_create(&threads[2], NULL, thread_func, "...");
	pthread_join(threads[0], NULL);

	pthread_join(threads[1], NULL);
	pthread_join(threads[2], NULL);
}
