#include <stdio.h>
//#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
//#include <pthread.h>
#define NTHREADS 10

void * thread_func(void * args){
        long long int k = 0;
        for(int i =0; i< 500000 ; i++){
                for(int j = 0 ; j < 10000; j++){
                         k = i*j;
                }
        }
}

int main()
{
	syscall(340,-1);
	//syscall()
//	int self_pid = getpid();
	
//	printf("system call returnedn %lf\n",(double) syscall(336,self_pid)/10e8);
        return 0;
}
