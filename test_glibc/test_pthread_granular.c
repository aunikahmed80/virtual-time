#include <stdio.h>
#include<pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/time.h>
#define NTHREADS 10
pthread_mutex_t lock;


/***********************Simulating******************************************
parent ***----------*--------********
thread1  ***********
thread2  *---------**********               
/***********************************************************************/
double time_diff(struct timeval x , struct timeval y)
{
        double x_ms , y_ms , diff;

        x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
        y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;

        diff = (double)y_ms - (double)x_ms;

        return diff;
}

void dummy(){
        long long int k = 0;

                for(int i =0; i< 5000 ; i++){
                        for(int j = 0 ; j < 10000; j++){
                                 k = i*j;
                        }
                }

}



void * thread_func(void * args){

        long long int k = 0;
        struct timeval before , after;
        double runtimedata[100];
        double time_sum = 0;
        for(int n=0; n < 100; n++){
                gettimeofday(&before , NULL);

       		 dummy();
       		 gettimeofday(&after , NULL);
       		 printf("dummy time elapsed : %.0lf\n " , time_diff(before , after));


                pthread_mutex_lock(&lock);
                gettimeofday(&before , NULL);
                for(int i =0; i< 100000000 ; i++){//5000
                        for(int j = 1000000 ; j < 1; j++){ //10000
				k = i*j;
			}
		}
		gettimeofday(&after , NULL);
		pthread_mutex_unlock(&lock);
	// int self_pid =(int) syscall(__NR_gettid);
        //unsigned long long int amma = syscall(333,self_pid);
        // printf("%d\tv_time returned %lf\n",self_pid,(double) syscall(333,self_pid)/10e8);
		printf("time elapsed in lock: %.0lf\n " , time_diff(before , after));
	}
}


int main()
{

        pthread_t threads[NTHREADS];
        pthread_mutex_init(&lock, NULL);

//      printf("%d\t%d\n",lock.__data.lock_acquisition_g_time,sizeof(lock));    
        pthread_create(&threads[0], NULL, thread_func, "...");

        pthread_create(&threads[1], NULL, thread_func, "...");
        pthread_create(&threads[2], NULL, thread_func, "...");
        pthread_join(threads[0], NULL);

        pthread_join(threads[1], NULL);
        pthread_join(threads[2], NULL);
        int self_pid =(int) syscall(__NR_gettid);

         printf("%d\tgroup v_time returned %lf\n",self_pid,(double) syscall(335,self_pid)/10e8);

        return 0;

}

