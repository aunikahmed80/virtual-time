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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "pthreadP.h"
#include <lowlevellock.h>
#include <stap-probe.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#ifndef lll_unlock_elision
#define lll_unlock_elision(a,b,c) ({ lll_unlock (a,c); 0; })
#endif

static int
internal_function
__pthread_mutex_unlock_full (pthread_mutex_t *mutex, int decr)
     __attribute_noinline__;

///////////////////////////////////////////////////////////////// Added function//////////////////////////////////////////////////////////////////
static void add_vtime(pthread_mutex_t *mutex){
       __v_t_list *temp = &mutex->__data.waiter_list;
	double delta_v = 0;
         while(temp->next !=NULL){
               if(mutex->__data.lock_acquisition_g_time >= temp->next->g_time_at_request){
			delta_v = (double)syscall(333,mutex->__data.__owner) -  mutex->__data.lock_acquisition_v_time;
			//printf("**In unlock**\towner:%d\trequest time is smaller, delta_v:%lf\t lock_accquisition_v_time:%lf \n",mutex->__data.__owner,delta_v/10e8,mutex->__data.lock_acquisition_v_time/10e8);
		}
		else{
			delta_v = (double)syscall(333,mutex->__data.__owner) - temp->next->lock_holder_v_time_at_request;
			//printf("Request time is larger, delta_v: %lld\n",delta_v);
		}
		printf("*In Unloack*\tadding %lf to thread: %d\n",delta_v, temp->next->tid);
		syscall(334,temp->next->tid,(long long int)delta_v); 
                temp = temp->next;
        }
}

/////////////////////////////////////////////////////////////////end of added function//////////////////////////////////////////////////////////

int
internal_function attribute_hidden
__pthread_mutex_unlock_usercnt (pthread_mutex_t *mutex, int decr)
{
  int type = PTHREAD_MUTEX_TYPE_ELISION (mutex);
  if (__builtin_expect (type &
		~(PTHREAD_MUTEX_KIND_MASK_NP|PTHREAD_MUTEX_ELISION_FLAGS_NP), 0))
    return __pthread_mutex_unlock_full (mutex, decr);

  if (__builtin_expect (type, PTHREAD_MUTEX_TIMED_NP)
      == PTHREAD_MUTEX_TIMED_NP)
    {
      /* Always reset the owner field.  */
    normal:
     add_vtime(mutex);	
      mutex->__data.__owner = 0;
      if (decr)
	/* One less user.  */
	--mutex->__data.__nusers;

      /* Unlock.  */
      lll_unlock (mutex->__data.__lock, PTHREAD_MUTEX_PSHARED (mutex));

      LIBC_PROBE (mutex_release, 1, mutex);

      return 0;
    }
  else if (__glibc_likely (type == PTHREAD_MUTEX_TIMED_ELISION_NP))
    {
      /* Don't reset the owner/users fields for elision.  */
      return lll_unlock_elision (mutex->__data.__lock, mutex->__data.__elision,
				      PTHREAD_MUTEX_PSHARED (mutex));
    }
  else if (__builtin_expect (PTHREAD_MUTEX_TYPE (mutex)
			      == PTHREAD_MUTEX_RECURSIVE_NP, 1))
    {
      /* Recursive mutex.  */
      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid))
	return EPERM;

      if (--mutex->__data.__count != 0)
	/* We still hold the mutex.  */
	return 0;
      goto normal;
    }
  else if (__builtin_expect (PTHREAD_MUTEX_TYPE (mutex)
			      == PTHREAD_MUTEX_ADAPTIVE_NP, 1))
    goto normal;
  else
    {
      /* Error checking mutex.  */
      assert (type == PTHREAD_MUTEX_ERRORCHECK_NP);
      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid)
	  || ! lll_islocked (mutex->__data.__lock))
	return EPERM;
      goto normal;
    }
}


static int
internal_function
__pthread_mutex_unlock_full (pthread_mutex_t *mutex, int decr)
{
  int newowner = 0;

  switch (PTHREAD_MUTEX_TYPE (mutex))
    {
    case PTHREAD_MUTEX_ROBUST_RECURSIVE_NP:
      /* Recursive mutex.  */
      if ((mutex->__data.__lock & FUTEX_TID_MASK)
	  == THREAD_GETMEM (THREAD_SELF, tid)
	  && __builtin_expect (mutex->__data.__owner
			       == PTHREAD_MUTEX_INCONSISTENT, 0))
	{
	  if (--mutex->__data.__count != 0)
	    /* We still hold the mutex.  */
	    return ENOTRECOVERABLE;

	  goto notrecoverable;
	}

      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid))
	return EPERM;

      if (--mutex->__data.__count != 0)
	/* We still hold the mutex.  */
	return 0;

      goto robust;

    case PTHREAD_MUTEX_ROBUST_ERRORCHECK_NP:
    case PTHREAD_MUTEX_ROBUST_NORMAL_NP:
    case PTHREAD_MUTEX_ROBUST_ADAPTIVE_NP:
      if ((mutex->__data.__lock & FUTEX_TID_MASK)
	  != THREAD_GETMEM (THREAD_SELF, tid)
	  || ! lll_islocked (mutex->__data.__lock))
	return EPERM;

      /* If the previous owner died and the caller did not succeed in
	 making the state consistent, mark the mutex as unrecoverable
	 and make all waiters.  */
      if (__builtin_expect (mutex->__data.__owner
			    == PTHREAD_MUTEX_INCONSISTENT, 0))
      notrecoverable:
	newowner = PTHREAD_MUTEX_NOTRECOVERABLE;

    robust:
      /* Remove mutex from the list.  */
      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
		     &mutex->__data.__list.__next);
      DEQUEUE_MUTEX (mutex);

      mutex->__data.__owner = newowner;
      if (decr)
	/* One less user.  */
	--mutex->__data.__nusers;

      /* Unlock.  */
      lll_robust_unlock (mutex->__data.__lock,
			 PTHREAD_ROBUST_MUTEX_PSHARED (mutex));

      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
      break;

    /* The PI support requires the Linux futex system call.  If that's not
       available, pthread_mutex_init should never have allowed the type to
       be set.  So it will get the default case for an invalid type.  */
#ifdef __NR_futex
    case PTHREAD_MUTEX_PI_RECURSIVE_NP:
      /* Recursive mutex.  */
      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid))
	return EPERM;

      if (--mutex->__data.__count != 0)
	/* We still hold the mutex.  */
	return 0;
      goto continue_pi_non_robust;

    case PTHREAD_MUTEX_PI_ROBUST_RECURSIVE_NP:
      /* Recursive mutex.  */
      if ((mutex->__data.__lock & FUTEX_TID_MASK)
	  == THREAD_GETMEM (THREAD_SELF, tid)
	  && __builtin_expect (mutex->__data.__owner
			       == PTHREAD_MUTEX_INCONSISTENT, 0))
	{
	  if (--mutex->__data.__count != 0)
	    /* We still hold the mutex.  */
	    return ENOTRECOVERABLE;

	  goto pi_notrecoverable;
	}

      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid))
	return EPERM;

      if (--mutex->__data.__count != 0)
	/* We still hold the mutex.  */
	return 0;

      goto continue_pi_robust;

    case PTHREAD_MUTEX_PI_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PI_NORMAL_NP:
    case PTHREAD_MUTEX_PI_ADAPTIVE_NP:
    case PTHREAD_MUTEX_PI_ROBUST_ERRORCHECK_NP:
    case PTHREAD_MUTEX_PI_ROBUST_NORMAL_NP:
    case PTHREAD_MUTEX_PI_ROBUST_ADAPTIVE_NP:
      if ((mutex->__data.__lock & FUTEX_TID_MASK)
	  != THREAD_GETMEM (THREAD_SELF, tid)
	  || ! lll_islocked (mutex->__data.__lock))
	return EPERM;

      /* If the previous owner died and the caller did not succeed in
	 making the state consistent, mark the mutex as unrecoverable
	 and make all waiters.  */
      if ((mutex->__data.__kind & PTHREAD_MUTEX_ROBUST_NORMAL_NP) != 0
	  && __builtin_expect (mutex->__data.__owner
			       == PTHREAD_MUTEX_INCONSISTENT, 0))
      pi_notrecoverable:
       newowner = PTHREAD_MUTEX_NOTRECOVERABLE;

      if ((mutex->__data.__kind & PTHREAD_MUTEX_ROBUST_NORMAL_NP) != 0)
	{
	continue_pi_robust:
	  /* Remove mutex from the list.
	     Note: robust PI futexes are signaled by setting bit 0.  */
	  THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending,
			 (void *) (((uintptr_t) &mutex->__data.__list.__next)
				   | 1));
	  DEQUEUE_MUTEX (mutex);
	}

    continue_pi_non_robust:
      mutex->__data.__owner = newowner;
      if (decr)
	/* One less user.  */
	--mutex->__data.__nusers;

      /* Unlock.  Load all necessary mutex data before releasing the mutex
	 to not violate the mutex destruction requirements (see
	 lll_unlock).  */
      int robust = mutex->__data.__kind & PTHREAD_MUTEX_ROBUST_NORMAL_NP;
      int private = (robust
		     ? PTHREAD_ROBUST_MUTEX_PSHARED (mutex)
		     : PTHREAD_MUTEX_PSHARED (mutex));
      if ((mutex->__data.__lock & FUTEX_WAITERS) != 0
	  || atomic_compare_and_exchange_bool_rel (&mutex->__data.__lock, 0,
						   THREAD_GETMEM (THREAD_SELF,
								  tid)))
	{
	  INTERNAL_SYSCALL_DECL (__err);
	  INTERNAL_SYSCALL (futex, __err, 2, &mutex->__data.__lock,
			    __lll_private_flag (FUTEX_UNLOCK_PI, private));
	}

      THREAD_SETMEM (THREAD_SELF, robust_head.list_op_pending, NULL);
      break;
#endif  /* __NR_futex.  */

    case PTHREAD_MUTEX_PP_RECURSIVE_NP:
      /* Recursive mutex.  */
      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid))
	return EPERM;

      if (--mutex->__data.__count != 0)
	/* We still hold the mutex.  */
	return 0;
      goto pp;

    case PTHREAD_MUTEX_PP_ERRORCHECK_NP:
      /* Error checking mutex.  */
      if (mutex->__data.__owner != THREAD_GETMEM (THREAD_SELF, tid)
	  || (mutex->__data.__lock & ~ PTHREAD_MUTEX_PRIO_CEILING_MASK) == 0)
	return EPERM;
      /* FALLTHROUGH */

    case PTHREAD_MUTEX_PP_NORMAL_NP:
    case PTHREAD_MUTEX_PP_ADAPTIVE_NP:
      /* Always reset the owner field.  */
    pp:
      mutex->__data.__owner = 0;

      if (decr)
	/* One less user.  */
	--mutex->__data.__nusers;

      /* Unlock.  */
      int newval, oldval;
      do
	{
	  oldval = mutex->__data.__lock;
	  newval = oldval & PTHREAD_MUTEX_PRIO_CEILING_MASK;
	}
      while (atomic_compare_and_exchange_bool_rel (&mutex->__data.__lock,
						   newval, oldval));

      if ((oldval & ~PTHREAD_MUTEX_PRIO_CEILING_MASK) > 1)
	lll_futex_wake (&mutex->__data.__lock, 1,
			PTHREAD_MUTEX_PSHARED (mutex));

      int oldprio = newval >> PTHREAD_MUTEX_PRIO_CEILING_SHIFT;

      LIBC_PROBE (mutex_release, 1, mutex);

      return __pthread_tpp_change_priority (oldprio, -1);

    default:
      /* Correct code cannot set any other type.  */
      return EINVAL;
    }

  LIBC_PROBE (mutex_release, 1, mutex);
  return 0;
}


int
__pthread_mutex_unlock (pthread_mutex_t *mutex)
{
  return __pthread_mutex_unlock_usercnt (mutex, 1);
}
strong_alias (__pthread_mutex_unlock, pthread_mutex_unlock)
hidden_def (__pthread_mutex_unlock)
