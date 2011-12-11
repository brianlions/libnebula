/*
 * async_event.h
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

#ifndef _BrianZ_NEBULA_ASYNC_EVENT_H_
#define _BrianZ_NEBULA_ASYNC_EVENT_H_

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <stdint.h>
#include "nebula/attributes.h"
#include "nebula/pretty_message.h"

namespace nebula
{
  class AsyncEvent
  {
  public:
    typedef enum
    {
      NONE = 0x00, READABLE = 0x01, WRITABLE = 0x02, RDWR = (READABLE | WRITABLE)
    } ae_event_type_t;

    typedef uint32_t (*ae_event_callback_t)(int fd, uint32_t mask, void * data, AsyncEvent * ae);
    typedef uint32_t (*ae_error_callback_t)(int fd, uint32_t mask_place_holder, void * data, AsyncEvent * ae);
    typedef uint32_t (*ae_time_callback_t)(uint64_t time_event_id, void * data, AsyncEvent * ae);

  private:
    enum
    {
      ERROR = -1, OK = 0
    };

    typedef struct AsyncFdEventInfo
    {
      uint32_t monitored_;
      uint32_t already_fired_;
      ae_event_callback_t proc_;
      ae_error_callback_t proc_exp_;
      void * data_;
      void * data_exp_;
    } ae_fd_event_info_t;

    typedef struct AsyncTimeEventInfo
    {
      uint64_t time_event_id_;
      int64_t when_ms_;
      ae_time_callback_t proc_;
      void * data_;
    } ae_time_event_info_t;

    int stop_;

    int epoll_fd_;
    int fd_table_size_;
    int num_monitored_fds_;
    int num_ready_fds_;
    int * list_of_ready_fds_;
    ae_fd_event_info_t * fd_event_list_;
    struct epoll_event * fired_events_;

    ae_time_event_info_t * time_event_list_;
    int size_time_events_;
    int used_time_events_;
    uint64_t last_time_event_id_;

  public:
    AsyncEvent() :
      stop_(0), //
          epoll_fd_(-1), fd_table_size_(0), num_monitored_fds_(0), num_ready_fds_(0), //
          list_of_ready_fds_(NULL), fd_event_list_(NULL), fired_events_(NULL), //
          time_event_list_(NULL), size_time_events_(0), used_time_events_(0), last_time_event_id_(0) //
    {
    }

    ~AsyncEvent()
    {
      if (epoll_fd_ >= 0) {
        close(epoll_fd_);
      }
      if (list_of_ready_fds_) {
        free(list_of_ready_fds_);
      }
      if (fd_event_list_) {
        free(fd_event_list_);
      }
      if (fired_events_) {
        free(fired_events_);
      }
      if (time_event_list_) {
        free(time_event_list_);
      }
    }

    int initialize(int epoll_hint_size = 64, int time_event_hint_size = 32) WARN_UNUSED_RESULT
    {
      if ((epoll_hint_size < 0) || (epoll_fd_ = epoll_create(epoll_hint_size)) < 0) {
        return ERROR;
      }
      fd_table_size_ = getdtablesize();
      if (!(list_of_ready_fds_ = (int *) calloc((size_t) fd_table_size_, sizeof(*list_of_ready_fds_)))) {
        return ERROR;
      }
      if (!(fd_event_list_ = (ae_fd_event_info_t *) calloc((size_t) fd_table_size_, sizeof(*fd_event_list_)))) {
        return ERROR;
      }
      if (!(fired_events_ = (epoll_event *) calloc((size_t) fd_table_size_, sizeof(*fired_events_)))) {
        return ERROR;
      }
      if ((size_time_events_ = time_event_hint_size) < 0 //
          || !(time_event_list_ = (ae_time_event_info_t *) calloc(size_time_events_, sizeof(*time_event_list_)))) {
        return ERROR;
      }

      DEV_MESSAGE("epoll_hint_size=%d,epoll_fd_=%d,fd_table_size_=%d,time_event_hint_size=%d",
          epoll_hint_size, epoll_fd_, fd_table_size_, time_event_hint_size);

      return OK;
    }

    /*
     * Description:
     *   Register a event callback function for event (or events) `mask' on `fd'.  Existing callback function
     *   for `fd' will be overwrite if successfully registered, buf will left untouched if failed.
     * Return value:
     *   0      ok
     *   <0     error
     */
    int addFdEvent(int fd, uint32_t mask, ae_event_callback_t proc, void *data = NULL,
      ae_error_callback_t proc_exp = (ae_error_callback_t) (0), void *data_exp = NULL) WARN_UNUSED_RESULT
    {
      if (fd >= fd_table_size_ || fd < 0) {
        return ERROR;
      }
      ae_fd_event_info_t *info = fd_event_list_ + fd;
      ae_fd_event_info_t old_info = *info; // save it, just in case
      int operation = info->monitored_ == NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
      info->monitored_ |= mask;
      info->proc_ = proc;
      info->data_ = data;
      info->proc_exp_ = proc_exp;
      info->data_exp_ = data_exp;
      struct epoll_event ee;
      memset(&ee, 0, sizeof(ee));
      ee.data.fd = fd;
      ee.events = EPOLLET;
      if (info->monitored_ & READABLE) {
        ee.events |= EPOLLIN;
      }
      if (info->monitored_ & WRITABLE) {
        ee.events |= EPOLLOUT;
      }

      DEV_MESSAGE("epoll_ctl(): epoll_fd_=%d,operation=%s,fd=%d,ee.data.fd=%d,ee.events=0x%x",
          epoll_fd_, operation == EPOLL_CTL_ADD ? "EPOLL_CTL_ADD" : "EPOLL_CTL_MOD",
          fd, ee.data.fd, ee.events);

      if (epoll_ctl(epoll_fd_, operation, fd, &ee) < 0) {
        *info = old_info; // restore to the original
        return ERROR;
      }
      if (operation == EPOLL_CTL_ADD) {
        ++num_monitored_fds_;
      }
      return OK;
    }

    /*
     * Description:
     *   Stops monitoring events `mask' on `fd'.
     * Return value:
     *   0      ok
     *   <0     error
     */
    int delFdEvent(int fd, uint32_t mask) WARN_UNUSED_RESULT
    {
      if (fd >= fd_table_size_ || fd < 0) {
        return ERROR;
      }
      if (mask == NONE) {
        return OK;
      }
      ae_fd_event_info_t *info = fd_event_list_ + fd;
      ae_fd_event_info_t old_info = *info; // save it, just in case
      info->monitored_ &= ~mask;
      struct epoll_event ee;
      memset(&ee, 0, sizeof(ee));
      ee.data.fd = fd;
      ee.events = EPOLLET;
      if (info->monitored_ & READABLE) {
        ee.events |= EPOLLIN;
      }
      if (info->monitored_ & WRITABLE) {
        ee.events |= EPOLLOUT;
      }
      if (info->monitored_ != NONE) {
        DEV_MESSAGE("epoll_ctl(): epoll_fd_=%d,operation=EPOLL_CTL_MOD,fd=%d,ee.data.fd=%d,ee.events=0x%x",
            epoll_fd_, fd, ee.data.fd, ee.events);

        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ee) < 0) {
          *info = old_info; // restore to the original
          return ERROR;
        }
        return OK;
      }
      else {
        DEV_MESSAGE("epoll_ctl(): epoll_fd_=%d,operation=EPOLL_CTL_DEL,fd=%d", epoll_fd_, fd);

        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ee) < 0) {
          *info = old_info;
          return ERROR;
        }
        memset(info, 0, sizeof(*info));
        --num_monitored_fds_;
        for (int i = 0; i < num_ready_fds_; ++i) {
          if (list_of_ready_fds_[i] == fd) {
            list_of_ready_fds_[i] = list_of_ready_fds_[--num_ready_fds_];
            break;
          }
        }
        return OK;
      }
    }

    /*
     * Description:
     *   Stops monitoring all future events on `fd', if `close_it' is true, `fd' will be close()ed by this method.
     * Notes:
     *   Do not close() `fd' before calling this method, or the internal state of the object will be wrong.
     * Return value:
     *   0      ok
     *   <0     error
     */
    int removeFd(int fd, bool close_it = false) WARN_UNUSED_RESULT
    {
      if (fd >= fd_table_size_ || fd < 0) {
        return ERROR;
      }
      ae_fd_event_info_t *info = fd_event_list_ + fd;
      if (info->monitored_ == NONE) {
        if (close_it) {
          close(fd);
        }
        return OK;
      }
      struct epoll_event ee;
      memset(&ee, 0, sizeof(ee));
      if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ee) < 0) {
        return ERROR;
      }
      if (close_it) {
        close(fd);
      }
      --num_monitored_fds_;
      memset(info, 0, sizeof(*info));
      for (int i = 0; i < num_ready_fds_; ++i) {
        if (list_of_ready_fds_[i] == fd) {
          list_of_ready_fds_[i] = list_of_ready_fds_[--num_ready_fds_];
          break;
        }
      }
      return OK;
    }

    /*
     * Description:
     *   Creates a time event that will be triggered `ms_later' milliseconds later, and stores ID of the newly
     *   created time event into `time_event_id' if it is not NULL.
     * Return value:
     *   0      ok
     *   <0     error
     */
    int addTimeEvent(uint32_t ms_later, ae_time_callback_t proc, void *data, uint64_t *time_event_id = NULL) WARN_UNUSED_RESULT
    {
      if (used_time_events_ == size_time_events_) {
        return ERROR;
      }
      (time_event_list_ + used_time_events_)->when_ms_ = msTimestampNow() + ms_later;
      (time_event_list_ + used_time_events_)->proc_ = proc;
      (time_event_list_ + used_time_events_)->data_ = data;
      (time_event_list_ + used_time_events_)->time_event_id_ = ++last_time_event_id_;
      ++used_time_events_;
      if (time_event_id) {
        *time_event_id = last_time_event_id_;
      }
      return OK;
    }

    /*
     * Description:
     *   Removes a previously created time event with the specified time event id.
     * Return value:
     *   OK if successfully removed, ERROR if no such time event was found.
     */
    int delTimeEvent(uint64_t time_event_id) WARN_UNUSED_RESULT
    {
      for (int i = 0; i < used_time_events_; ++i) {
        if ((time_event_list_ + i)->time_event_id_ == time_event_id) {
          *(time_event_list_ + i) = *(time_event_list_ + (--used_time_events_));
          return OK;
        }
      }

      return ERROR;
    }

    void start()
    {
      stop_ = 0;
    }

    void stop()
    {
      stop_ = 1;
    }

    int fdTableSize() const
    {
      return fd_table_size_;
    }

    int numMonitoredFds() const
    {
      return num_monitored_fds_;
    }

    int numReadyFds() const
    {
      return num_ready_fds_;
    }

    int timeEventTableSize() const
    {
      return size_time_events_;
    }

    int numTimeEvents() const
    {
      return used_time_events_;
    }

    uint64_t lastTimeEventId() const
    {
      return last_time_event_id_;
    }

    void eventLoop()
    {
      DEV_MESSAGE("entering eventLoop() - stop_=%d,epoll_fd_=%d,"
          "fd_table_size_=%d,num_monitored_fds_=%d,num_ready_fds_=%d,"
          "size_time_events_=%d,used_time_events_=%d,last_time_event_id_=%lu",
          stop_, epoll_fd_,
          fd_table_size_, num_monitored_fds_, num_ready_fds_,
          size_time_events_, used_time_events_, last_time_event_id_);

      int ms_left;
      while (!stop_ && (num_monitored_fds_ > 0 || used_time_events_ > 0)) {
        DEV_MESSAGE("in eventLoop() - stop_=%d,epoll_fd_=%d,"
            "fd_table_size_=%d,num_monitored_fds_=%d,num_ready_fds_=%d,"
            "size_time_events_=%d,used_time_events_=%d,last_time_event_id_=%lu",
            stop_, epoll_fd_,
            fd_table_size_, num_monitored_fds_, num_ready_fds_,
            size_time_events_, used_time_events_, last_time_event_id_);

        processReadyFds();
        ms_left = static_cast<int> (nearestTimeEvent());

        DEV_MESSAGE("nearest time event in %d ms, num_ready_fds_=%d", ms_left, num_ready_fds_);

        processFdEvents((ms_left == 0) ? 0 : (num_ready_fds_ > 0 ? 1 : ms_left));
        processTimeEvents();
      }

      DEV_MESSAGE("leaving eventLoop()");
    }

  private:
    /*
     * Description:
     *   Processes triggered time events.
     * Return value:
     *   Number of time events processed during this round.
     */
    int processTimeEvents()
    {
      int64_t now = msTimestampNow();
      uint32_t rc;
      int i = 0, processed = 0;
      while (i < used_time_events_) {
        if (time_event_list_[i].when_ms_ <= now) {
          DEV_MESSAGE("handling time event: time_event_id=%lu", time_event_list_[i].time_event_id_);

          ++processed;
          if ((rc = time_event_list_[i].proc_(time_event_list_[i].time_event_id_, time_event_list_[i].data_, this))) {
            time_event_list_[i].when_ms_ = now + static_cast<int64_t> (rc);
          }
          else {
            time_event_list_[i] = time_event_list_[--used_time_events_];
            continue;
          }
        }
        ++i;
      }
      return processed;
    }

    /*
     * Description:
     *   Waits for events on monitored file descriptors.
     * Return value:
     *   Number of fds processed, 0 if timed out, -1 if error.
     */
    int processFdEvents(int timeout_ms)
    {
      int nr = epoll_wait(epoll_fd_, fired_events_, fd_table_size_, timeout_ms);
      if (nr > 0) {
        ae_fd_event_info_t * info;
        uint32_t events;
        uint32_t ae_events;
        struct epoll_event * ee;
        uint32_t guess = 0;
        for (int i = 0; i < nr; ++i) {
          ae_events = NONE;
          ee = fired_events_ + i;
          events = ee->events;
          info = fd_event_list_ + ee->data.fd;

          DEV_MESSAGE("handling epoll events: ee->data.fd=%d,ee->events=0x%x(%d)",
              ee->data.fd, ee->events, ee->events);

          if (events & EPOLLIN) {
            events &= ~EPOLLIN;
            ae_events |= READABLE;
          }
          if (events & EPOLLOUT) {
            events &= ~EPOLLOUT;
            ae_events |= WRITABLE;
          }
          // first handle exceptions
          if ((events & (EPOLLHUP | EPOLLERR)) && info->proc_exp_) {
            DEV_MESSAGE("handling exceptions: ee->data.fd=%d, events=0x%x(%d)",
                ee->data.fd, events, events);
            events &= ~(EPOLLHUP | EPOLLERR);
            // TODO second argument is a place holder
            info->proc_exp_(ee->data.fd, 0, info->data_exp_, this);
          }
          // then handle I/O events if no exception was occured
          else if (ae_events != NONE) {
            DEV_MESSAGE("handling i/o events: ee->data.fd=%d, ae_events=0x%x(%d)",
                ee->data.fd, ae_events, ae_events);
#if 1
            guess = info->proc_(ee->data.fd, ae_events, info->data_, this);
            guess |= info->already_fired_;
            guess &= info->monitored_;
            if (guess) {
              if (!info->already_fired_) {
                list_of_ready_fds_[num_ready_fds_++] = ee->data.fd;
              }
              info->already_fired_ |= guess;
            }
            else {
              if (info->already_fired_) { // pending events
                info->already_fired_ &= ~ae_events;
                if (!info->already_fired_) {
                  for (int i = 0; i < num_ready_fds_; ++i) {
                    if (list_of_ready_fds_[i] == ee->data.fd) {
                      list_of_ready_fds_[i] = list_of_ready_fds_[--num_ready_fds_];
                      break;
                    }
                  }
                }
              }
              else {
                // not needed, remove later
              }
            }
#elif 1
            if ((guess = info->proc_(ee->data.fd, ae_events, info->data_, this)) && (guess
                    &= info->monitored_) != 0) {
              if (!info->already_fired_) {
                ready_fd_list_[ready_fds_++] = ee->data.fd;
              }
              info->already_fired_ |= guess;
            }
            else if (info->already_fired_ && !(info->already_fired_ &= ~ae_events)) {
              for (int i = 0; i < ready_fds_; ++i) {
                if (ready_fd_list_[i] == ee->data.fd) {
                  ready_fd_list_[i] = ready_fd_list_[--ready_fds_];
                  break;
                }
              }
            }
#else
            if ((guess = info->proc_(ee->data.fd, ae_events, info->data_, this)) && (guess
                    & info->monitored_)) {
              info->already_fired_ |= (guess & info->monitored_);
              ready_fd_list_[ready_fds_++] = ee->data.fd;
            }
#endif
            DEV_MESSAGE("finished handling i/o events: ee->data.fd=%d, ae_events=0x%x(%d)",
                ee->data.fd, ae_events, ae_events);
          }
          if (events) {
            // FIXME what if there's other events not handled?
            DEV_MESSAGE("some events not handled: ee->data.fd=%d, events=0x%x(%d)",
                ee->data.fd, events, events);
          }
        }
      }
      else if (nr == 0) {
      }
      else {
      }
      return nr;
    }

    /*
     * Description:
     *   Since we're using the ET interface of epoll, we have to periodically process I/O requests on those
     *   file descriptors.
     * Return value:
     *   Number of file descriptors processed.
     */
    int processReadyFds()
    {
      int processed = 0;
      int idx = 0;
      uint32_t guess;
      while (idx < num_ready_fds_) {
        int fd = list_of_ready_fds_[idx];
        ae_fd_event_info_t *info = fd_event_list_ + fd;
#if 1
        ++processed;
        guess = info->proc_(fd, info->already_fired_, info->data_, this);
        guess |= info->already_fired_;
        guess &= info->monitored_;
        if (guess) {
          info->already_fired_ = guess;
          ++idx;
        }
        else {
          info->already_fired_ = 0;
          if (list_of_ready_fds_[idx] == fd) { // in case this `fd' was already removed by the callback function
            list_of_ready_fds_[idx] = list_of_ready_fds_[--num_ready_fds_];
          }
        }
#elif 1
        ++processed;
        if ((guess = info->proc_(fd, info->already_fired_, info->data_, this)) && (guess &= info->monitored_)
            != 0) {
          info->already_fired_ = guess;
          ++idx;
        }
        else {
          info->already_fired_ = 0;
          ready_fd_list_[idx] = ready_fd_list_[--ready_fds_];
        }
#else
        guess = info->proc_(fd, info->already_fired_, info->data_, this);
        DEV_MESSAGE("events already fired handled: fd=%d, guess=0x%x(%d)", fd, guess, guess);
        ++processed;
        info->already_fired_ = (guess & info->monitored_);
        if (!guess) {
          ready_fd_list_[idx] = ready_fd_list_[--ready_fds_];
          continue;
        }
        ++idx;
#endif
      }
      return processed;
    }

    /*
     * Description:
     *   Find out the nearest time event, and compute number of milliseconds left before firing the time event.
     * Return value:
     *   Number of milliseconds left, or -1 if no future time event.
     */
    int64_t nearestTimeEvent()
    {
      int64_t ms_left = -1;
      int idx = -1;
      for (int i = 0; i < used_time_events_; ++i) {
        if (idx < 0 || time_event_list_[i].when_ms_ < time_event_list_[idx].when_ms_) {
          idx = i;
        }
      }
      if (idx >= 0 && (ms_left = time_event_list_[idx].when_ms_ - msTimestampNow()) < 0) {
        // oops, we missed the time event
        ms_left = 0;
      }
      return ms_left;
    }

    int64_t msTimestampNow()
    {
      struct timeval now;
      gettimeofday(&now, NULL);
      return ((int64_t) now.tv_sec) * 1000 + ((int64_t) now.tv_usec) / 1000;
    }
  };
}

#endif /* _BrianZ_NEBULA_ASYNC_EVENT_H_ */
