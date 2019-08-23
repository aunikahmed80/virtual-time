/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>

#include <atomic.h>
#include "pthreadP.h"

#include <stap-probe.h>
#include<stdio.h>

#include <sys/syscall.h>
#include <unistd.h>

static void
cleanup (void *arg)
{
  /* If we already changed the waiter ID, reset it.  The call cannot
     fail for any reason but the thread not having done that yet so
     there is no reason for a loop.  */
  (void) atomic_compare_and_exchange_bool_acq ((struct pthread **) arg, NULL,
					       THREAD_SELF);
}


int
pthread_join (pthread_t threadid, void **thread_return)
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (INVALID_NOT_TERMINATED_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

  /* Is the thread joinable?.  */
  if (IS_DETACHED (pd))
    /* We cannot wait for the thread.  */
    return EINVAL;
 int tid = pd->tid; ////////////////////////////Ahmed////////////////////////////////

  struct pthread *self = THREAD_SELF;
  int result = 0;

  LIBC_PROBE (pthread_join, 1, threadid);

  /* During the wait we change to asynchronous cancellation.  If we
     are canceled the thread we are waiting for must be marked as
     un-wait-ed for again.  */
  pthread_cleanup_push (cleanup, &pd->joinid);

  /* Switch to asynchronous cancellation.  */
  int oldtype = CANCEL_ASYNC ();
//  int tid = 0;
  if ((pd == self
       || (self->joinid == pd
	   && (pd->cancelhandling
	       & (CANCELING_BITMASK | CANCELED_BITMASK | EXITING_BITMASK
		  | TERMINATED_BITMASK)) == 0))
      && !CANCEL_ENABLED_AND_CANCELED (self->cancelhandling))
    /* This is a deadlock situation.  The threads are waiting for each
       other to finish.  Note that this is a "may" error.  To be 100%
       sure we catch this error we would have to lock the data
       structures but it is not necessary.  In the unlikely case that
       two threads are really caught in this situation they will
       deadlock.  It is the programmer's problem to figure this
       out.  */
    result = EDEADLK;
  /* Wait for the thread to finish.  If it is already locked something
     is wrong.  There can only be one waiter.  */
  else if (__builtin_expect (atomic_compare_and_exchange_bool_acq (&pd->joinid,
								   self,
								   NULL), 0))
    /* There is already somebody waiting for the thread.  */
    result = EINVAL;
  else{
    /* Wait for the child.  */

 //int self_pid =(int) syscall(__NR_gettid);
 //tid = pd->tid;
//printf("pthread_join called. self pid: self_pid:%d,\tchild_pid:%d\n",self_pid,pd->tid);
    lll_wait_tid (pd->tid);
}
//////////////////////////////////////////////////////////////////////////////Ahmed code///////////////////////////////////////////////////////
/*
  unsigned long long int child_vtime = (unsigned long long int)syscall(333,tid);
  int self_pid =(int) syscall(__NR_gettid);
  unsigned long long int self_vtime = (unsigned long long int)syscall(333,self_pid);

	printf("pthread_join called. self pid: self_pid:%d,\tchild vtime:%llu\n",self_pid,child_vtime);
  if (child_vtime > self_vtime){
	printf("child vtime is greater and diff is:%llu\n",child_vtime- self_vtime);
	syscall(338,self_pid,(long long int)child_vtime);
 	}
*/
	printf("pthread_join called with child id: %d",tid);
	syscall(340,tid);
/////////////////////////////////////////////////////////////////////////////////End//////////////////////////////////////////////////////////

  /* Restore cancellation mode.  */
  CANCEL_RESET (oldtype);

  /* Remove the handler.  */
  pthread_cleanup_pop (0);


  if (__glibc_likely (result == 0))
    {
      /* We mark the thread as terminated and as joined.  */
      pd->tid = -1;

      /* Store the return value if the caller is interested.  */
      if (thread_return != NULL)
	*thread_return = pd->result;


      /* Free the TCB.  */
      __free_tcb (pd);
    }

  LIBC_PROBE (pthread_join_ret, 3, threadid, result, pd->result);

  return result;
}
