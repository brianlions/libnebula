/*
 * thread.h
 *
 *  Created on: Dec 9, 2011
 *  Author:     Brian Y. ZHANG
 *  Email:      brianlions at gmail dot com
 */
/*
 * Copyright (c) 2011 Brian Yi ZHANG <brianlions at gmail dot com>
 *
 * This file is part of libnebula.
 *
 * libnebula is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libnebula is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libnebula.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BrianZ_NEBULA_THREAD_H_
#define _BrianZ_NEBULA_THREAD_H_

#include <pthread.h>
#include <string.h>

namespace nebula
{
  class Thread
  {
  private:
    volatile int stop_;
    pthread_t thread_id_;
    pthread_attr_t thread_attr_;

  public:
    Thread()
    {
      stop_ = 0;
      memset(&thread_id_, 0, sizeof(thread_id_));
      if (pthread_attr_init(&thread_attr_)) {
        // FIXME
      }
    }

    virtual ~Thread()
    {
      if (pthread_attr_destroy(&thread_attr_)) {
        // FIXME
      }
    }

    /*
     * Description:
     *  Start handler of `this' thread, called immediately after `this' thread is created.
     */
    virtual void startHandler()
    {

    }

    /*
     * Description:
     *   Exit handler of `this' thread, called before `this' thread's termination.
     */
    virtual void exitHandler()
    {

    }

    /*
     * Description:
     *   This is the start routine of `this' thread.
     * Return value:
     *   The return value is the exit status of `this' thread.
     */
    virtual void * routine() = 0;

    /*
     * Description:
     *   Start a new thread.
     */
    int create()
    {
      return pthread_create(&thread_id_, &thread_attr_, privateStartRoutine, this);
    }

    /*
     * Description:
     *   Waits for thread `this' to terminate, exit status of thread `this'
     *   will be copied into `*retval'.  If thread `this' was canceled, then
     *   PTHREAD_CANCELED is placed into `*retval'.
     */
    int join(void ** retval = NULL)
    {
      return pthread_join(thread_id_, retval);
    }

    /*
     * Description:
     *   Send a cancellation request to `this' thread.
     */
    int cancel()
    {
      return pthread_cancel(thread_id_);
    }

    /*
     * Description:
     *   Obtain ID of `this' thread.
     */
    pthread_t self() const
    {
      return thread_id_;
    }

    /*
     * Description:
     *   Send a signal to `this' thread.
     */
    int kill(int sig = 0)
    {
      return pthread_kill(thread_id_, sig);
    }

    int trySetStopFlag()
    {
      return __sync_val_compare_and_swap(&stop_, 0, 1);
    }

    void setStopFlag()
    {
      while (!__sync_val_compare_and_swap(&stop_, 0, 1)) {
        ;
      }
    }

    int getStopFlag()
    {
      return __sync_fetch_and_add(&stop_, 0);
    }

  protected:
    /*
     * Template method of thread
     */
    void * startRoutine()
    {
      startHandler();
      void * result = routine();
      exitHandler();
      return result;
    }

  private:
    static void * privateStartRoutine(void * self)
    {
#if 0
      return reinterpret_cast<Thread*> (self)->routine();
#else
      return reinterpret_cast<Thread*> (self)->startRoutine();
#endif
    }
  }; /* class Thread */
}

#endif /* _BrianZ_NEBULA_THREAD_H_ */
