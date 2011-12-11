/*
 * mutex.h
 *
 *  Created on: Dec 3, 2011
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

#ifndef _BrianZ_NEBULA_MUTEX_H_
#define _BrianZ_NEBULA_MUTEX_H_

#include <pthread.h>

namespace nebula
{
  class Mutex
  {
  private:
    pthread_mutex_t impl_;

  public:
    Mutex()
    {
      int ec = pthread_mutex_init(&impl_, NULL);
      if (ec) {
        // FIXME
      }
    }

    ~Mutex()
    {
      pthread_mutex_destroy(&impl_);
    }

    void lock()
    {
      pthread_mutex_lock(&impl_);
    }

    bool try_lock()
    {
      return pthread_mutex_trylock(&impl_) == 0;
    }

    void unlock()
    {
      pthread_mutex_unlock(&impl_);
    }

    typedef pthread_mutex_t * native_handle_type;

    native_handle_type nativeHandle()
    {
      return (native_handle_type) &impl_;
    }
  };
}

#endif /* _BrianZ_NEBULA_MUTEX_H_ */
