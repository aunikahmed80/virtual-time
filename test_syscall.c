#include <stdio.h>
//#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
int main()
{		long long  int  k = 0; 
	 int self_pid = getpid();
	for (int i = 0 ; i <100000000; i++)
		for (int j; j < 100000000; j++)
			k++;
double d = syscall(333,self_pid);
	printf("Before adding time:%lf\n",d);
	double amma = syscall(338,self_pid,100000000);
        printf("pid:%d\tafter setting vtime  %lf\n",self_pid, amma);
        amma = syscall(335,self_pid);
        printf("pid:%d\tgroup v_time  %lf\n",self_pid, amma);
         return 0;
}
