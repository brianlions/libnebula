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

    virtual void * routine() = 0;

    int create()
    {
      return pthread_create(&thread_id_, &thread_attr_, privateStartRoutine, this);
    }

    int join(void ** retval = NULL)
    {
      return pthread_join(thread_id_, retval);
    }

    pthread_t self() const
    {
      return thread_id_;
    }

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

  private:
    static void * privateStartRoutine(void * self)
    {
      return reinterpret_cast<Thread*> (self)->routine();
    }
  }; /* class Thread */
}

#endif /* _BrianZ_NEBULA_THREAD_H_ */
