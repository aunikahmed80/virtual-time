#include <stdio.h>
#include <pthread.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<unistd.h>

#define NTHREADS 10



void * thread_func(void * args){
	double k;	
	for(int i =0; i< 10000000 ; i++){
		for(int j = 0 ; j < 10000; j++){
			 k = i*j;
		}
	}
}

int main()
{
  

pthread_t threads[NTHREADS];


  for (int count = 0; count < NTHREADS; ++count)
    {
      pthread_create(&threads[count], NULL, thread_func, "...");
	}
  
for (int i = 0; i < NTHREADS; ++i)
    {
      pthread_join(threads[i], NULL);
    }

	return 0;

}
