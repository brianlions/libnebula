/*
 * time.h
 *
 *  Created on: Nov 29, 2011
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

#ifndef _BrianZ_NEBULA_TIME__H_
#define _BrianZ_NEBULA_TIME__H_

#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "nebula/attributes.h"

namespace nebula
{
  class Time
  {
  public:
    static char * formatDataTime(char * buf, size_t buflen, const char * format, const time_t * pt = NULL,
      bool utc = false) STRFTIME_FORMAT(3)
    {
      struct tm broken_down;
      time_t now;
      if (!pt) {
        time(&now);
        pt = &now;
      }
      utc ? gmtime_r(pt, &broken_down) : localtime_r(pt, &broken_down);
      strftime(buf, buflen, format, &broken_down);
      return buf;
    }

    static char * fromTimestamp(char * buf, size_t buflen, const time_t * pt = NULL, bool utc = false)
    {
      struct tm broken_down;
      time_t now;
      if (!pt) {
        time(&now);
        pt = &now;
      }
      utc ? gmtime_r(pt, &broken_down) : localtime_r(pt, &broken_down);
      strftime(buf, buflen, "%Y.%m.%d-%H:%M:%S-%Z", &broken_down);
      return buf;
    }

    static char * strTimestamp(char * buf, size_t buflen, const struct timeval * ptv = NULL, bool utc = false)
    {
      struct tm broken_down;
      struct timeval now;

      if (!ptv) {
        gettimeofday(&now, NULL);
        ptv = &now;
      }
      utc ? gmtime_r(&(ptv->tv_sec), &broken_down) : localtime_r(&(ptv->tv_sec), &broken_down);

      char format[64];
      sprintf(format, "%%Y.%%m.%%d-%%H:%%M:%%S.%06d-%%Z", static_cast<int> (ptv->tv_usec));
      strftime(buf, buflen, format, &broken_down);
      return buf;
    }

    static int64_t msTimestamp(const struct timeval * ptv = NULL)
    {
      struct timeval now;
      if (!ptv) {
        gettimeofday(&now, NULL);
        ptv = &now;
      }
      return static_cast<int64_t> (ptv->tv_sec * 1000 + ptv->tv_usec / 1000);
    }

    static int64_t usTimestamp(const struct timeval * ptv = NULL)
    {
      struct timeval now;
      if (!ptv) {
        gettimeofday(&now, NULL);
        ptv = &now;
      }
      return static_cast<int64_t> (ptv->tv_sec * 1000000 + ptv->tv_usec);
    }

    static int msSleep(unsigned int interval, unsigned int * remain = NULL)
    {
      struct timespec intv, rem;
      intv.tv_sec = (time_t) interval / 1000;
      intv.tv_nsec = (long int) (interval % 1000) * 1000000;
      if (nanosleep(&intv, remain ? &rem : NULL) < 0) {
        if (errno == EINTR && remain) {
          *remain = static_cast<unsigned int> (rem.tv_sec * 1000 + rem.tv_nsec / 1000000);
        }
        return -1;
      }
      else {
        if (remain) {
          *remain = 0;
        }
        return 0;
      }
    }
  };

  class StopWatch
  {
  private:
    struct timeval start_time_, stop_time_;
    bool started_, stopped_;

    mutable char * message_buf_;
    mutable size_t buf_size_;
    enum
    {
      default_buf_size = 128
    };

  public:
    StopWatch() :
      started_(false), stopped_(false), message_buf_(NULL), buf_size_(0)
    {
      start_time_.tv_sec = stop_time_.tv_sec = 0;
      start_time_.tv_usec = stop_time_.tv_usec = 0;
    }

    virtual ~StopWatch()
    {
      if (message_buf_) {
        free(message_buf_);
      }
    }

    void start()
    {
      started_ = true;
      stopped_ = false;
      gettimeofday(&start_time_, NULL);
    }

    void stop()
    {
      gettimeofday(&stop_time_, NULL);
      stopped_ = true;
    }

    int64_t timeCostUs() const
    {
      if (started_ && stopped_) {
        return static_cast<int64_t> ((stop_time_.tv_sec - start_time_.tv_sec) * 1000000 + (stop_time_.tv_usec
            - start_time_.tv_usec));
      }
      return 0;
    }

    void reset()
    {
      started_ = stopped_ = false;
    }

    const char * message(const char * msg_name, int64_t num_iterations) const
    {
      if (!msg_name || !num_iterations) {
        return "invalid parameter";
      }

      if (!message_buf_) {
        if (!(message_buf_ = (char *) malloc(default_buf_size))) {
          return "malloc() failed";
        }
        buf_size_ = default_buf_size;
      }

      int used = 0;
      if ((used = snprintf(message_buf_, buf_size_, "%s: %ld usec / %ld = %0.6f usec", msg_name, timeCostUs(),
        num_iterations, 1.0 * timeCostUs() / num_iterations)) >= buf_size_) {
        char * temp = (char *) realloc(message_buf_, (size_t) used);
        if (!temp) {
          return "realloc() failed";
        }
        message_buf_ = temp;
        buf_size_ = (size_t) used;
        snprintf(message_buf_, buf_size_, "%s: %ld usec / %ld = %0.6f usec", msg_name, timeCostUs(), num_iterations,
          1.0 * timeCostUs() / num_iterations);
      }

      return message_buf_;
    }
  };
}

#endif /* _BrianZ_NEBULA_TIME__H_ */
