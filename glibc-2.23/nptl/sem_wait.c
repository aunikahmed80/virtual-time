/* sem_wait -- wait on a semaphore.  Generic futex-using version.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <lowlevellock.h>	/* lll_futex* used by the old code.  */
#include "sem_waitcommon.c"

#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
/***************************************************************Ahmed code*************************************/

static void enqueue_tid(struct new_sem* semaphore, int tid){
        
	int expected_num_thread = 20;
	vtime_node *new_node = (vtime_node*)malloc(sizeof(vtime_node));
        new_node->tid = tid;
        
        new_node->times_at_request = (unsigned long *)malloc((expected_num_thread+1)*3*sizeof(unsigned long));
        syscall(339,expected_num_thread,  new_node->times_at_request, tid);
        
	new_node->next = semaphore->waiter_list.next;
        new_node->prev = &semaphore->waiter_list ;
        if(new_node->next != NULL){//Has previous item
                new_node->next->prev = new_node;
        }
        semaphore->waiter_list.next = new_node;

}


static void add_waiting_time(struct new_sem* semaphore, int tid){
//printf("Dequeue called with %d\n",tid);

	//remove node from link_list

        vtime_node *temp = &semaphore->waiter_list;
	vtime_node *node_2b_deleted = NULL;
                while(temp->next !=NULL){
               if(temp->next->tid == tid){
			node_2b_deleted = temp->next;
                //      printf("Found match in dequeue %d\n",tid);
                        if(temp->next->next!= NULL){ // more nodes after it 
                                //printf("enter if !");
			
                                temp->next->next->prev = temp;
                                //printf("if executed!\n");
                        }
                        temp->next = temp->next->next;
                        //printf("temp next executed!\n");      
			 break;
                }
                temp = temp->next;
        }

	//Add waiting time to the waiter
	int i;
	double waiting_time = 0 ;
	for(i= 1; i<node_2b_deleted->times_at_request[0]; i++){ // 0th index contain the number of thread running at the wait call by this thread
		if (node_2b_deleted->times_at_request[3*i]==semaphore->latest_post_from){
			waiting_time = semaphore->latest_time_at_posting - node_2b_deleted->times_at_request[3*i+1];
			break;	
		}
	
	}
        free(node_2b_deleted);
	printf("waiting time for semaphore:%lf\n",waiting_time);
	syscall(334, tid, (long long int)waiting_time);
                       

}



/***************************************************************End********************************************/













int
__new_sem_wait (sem_t *sem)
{
  if (__new_sem_wait_fast ((struct new_sem *) sem, 0) == 0)
    return 0;
  else{
	int self_pid =(int) syscall(__NR_gettid);
	struct new_sem* semaphore =(struct new_sem *) sem;
	enqueue_tid(semaphore,self_pid);
    	
	int err = __new_sem_wait_slow(semaphore, NULL);
	add_waiting_time(semaphore, self_pid);
	return err;
	

     }
}
versioned_symbol (libpthread, __new_sem_wait, sem_wait, GLIBC_2_1);

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)
int
attribute_compat_text_section
__old_sem_wait (sem_t *sem)
{
  int *futex = (int *) sem;
  int err;

  do
    {
      if (atomic_decrement_if_positive (futex) > 0)
	return 0;

      /* Enable asynchronous cancellation.  Required by the standard.  */
      int oldtype = __pthread_enable_asynccancel ();

      /* Always assume the semaphore is shared.  */
      err = lll_futex_wait (futex, 0, LLL_SHARED);

      /* Disable asynchronous cancellation.  */
      __pthread_disable_asynccancel (oldtype);
    }
  while (err == 0 || err == -EWOULDBLOCK);

  __set_errno (-err);
  return -1;
}

compat_symbol (libpthread, __old_sem_wait, sem_wait, GLIBC_2_0);
#endif

int
__new_sem_trywait (sem_t *sem)
{
  /* We must not fail spuriously, so require a definitive result even if this
     may lead to a long execution time.  */
  if (__new_sem_wait_fast ((struct new_sem *) sem, 1) == 0)
    return 0;
  __set_errno (EAGAIN);
  return -1;
}
versioned_symbol (libpthread, __new_sem_trywait, sem_trywait, GLIBC_2_1);
#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)
int
__old_sem_trywait (sem_t *sem)
{
  int *futex = (int *) sem;
  int val;

  if (*futex > 0)
    {
      val = atomic_decrement_if_positive (futex);
      if (val > 0)
	return 0;
    }

  __set_errno (EAGAIN);
  return -1;
}
compat_symbol (libpthread, __old_sem_trywait, sem_trywait, GLIBC_2_0);
#endif
