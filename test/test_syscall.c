#include <stdio.h>
//#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define NTHREADS 10

/*void test_func(){

	unsigned long** p =(unsigned long**) malloc(5*sizeof(unsigned long**));
	for (int i=0; i<5;i++){
		unsigned long* temp =(unsigned long*) malloc(2*sizeof(unsigned long*));
		temp[0] =i* 1;
		temp[1] =i* 2;
		p[i] = temp;
		 
	}
	for (int i=0; i<5;i++){
		unsigned long* temp = p[i];
		printf("%lu\t%lu\n",temp[0],temp[1]);
		 
	}


}
*/

void * thread_func(void * args){
        long long int k = 0;
        for(int i =0; i< 500000 ; i++){
                for(int j = 0 ; j < 10000; j++){
                         k = i*j;
                }
        }
}

int main()
{		/*long long  int  k = 0; 
	 int self_pid = getpid();
	for (int i = 0 ; i <100000000; i++)
		for (int j; j < 100000000; j++)
			k++;
double d = syscall(333,self_pid);
	printf("Before adding time:%lf\n",d);
	double amma = syscall(338,self_pid,100000000);
        printf("pid:%d\tafter setting vtime  %lf\n",self_pid, amma);*/
 pthread_t threads[NTHREADS];

//      printf("%d\t%d\n",lock.__data.lock_acquisition_g_time,sizeof(lock));    
        pthread_create(&threads[0], NULL, thread_func, "...");
     	int expected_num_thread = 9;	
	unsigned long *p = (unsigned long *)malloc((expected_num_thread+1)*3*sizeof(unsigned long));
	//unsigned long p [(expected_num_thread+1)*3];	
	int self_pid = getpid();
	sleep(1);
        syscall(339,expected_num_thread,p,self_pid);
	int k = (int) p[0];
	printf("Num thread:%d\n",k);
	for( int i =3;i<=3*k;i+=3)
  	     	printf("pid:%lu\t  v_time  %lu \t gtime:%lu\n", p[i],p[i+1],p[i+2]);
	free(p);
	pthread_join(threads[0], NULL);	
//	test_func();
        return 0;
}
