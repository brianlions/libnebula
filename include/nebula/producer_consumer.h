/*
 * producer_consumer.h
 *
 *  Created on: Dec 14, 2011
 *  Author:     Brian Y. ZHANG
 *  Email:      brianzhang at gmail dot com
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

#ifndef _BrianZ_NEBULA_PRODUCER_CONSUMER_H_
#define _BrianZ_NEBULA_PRODUCER_CONSUMER_H_

#include <pthread.h>
#include <list>
#include <sstream>
#include "nebula/attributes.h"
#include "nebula/thread.h"

namespace nebula
{
  namespace ProducerAndConsumer
  {
    class Tasklet
    {
    public:
      struct Result
      {
        enum Constants
        {
          NOT_AVAILABLE = 0, SUCCESS = 1, FAILED = 2, ABORTED = 3, CANCELLED = 4
        };

        static const char * toString(Constants v)
        {
          switch (v) {
            case NOT_AVAILABLE:
              return "NOT_AVAILABLE";
              break;
            case SUCCESS:
              return "SUCCESS";
              break;
            case FAILED:
              return "FAILED";
              break;
            case ABORTED:
              return "ABORTED";
              break;
            case CANCELLED:
              return "CANCELLED";
              break;
            default:
              return "<unknown>";
              break;
          }
        }
      };

    private:
      Result::Constants result_;

    public:
      Tasklet() :
        result_(Result::NOT_AVAILABLE)
      {

      }

      virtual ~Tasklet()
      {

      }

      Result::Constants getResult() const
      {
        return result_;
      }

      const char * getResultStr() const
      {
        return Result::toString(result_);
      }

      void execute()
      {
        switch (process()) {
          case Result::NOT_AVAILABLE:
            result_ = Result::NOT_AVAILABLE;
            reportResult(result_);
          case Result::SUCCESS:
            result_ = Result::SUCCESS;
            reportResult(result_);
            break;
          case Result::FAILED:
            result_ = Result::FAILED;
            reportResult(result_);
            break;
          case Result::ABORTED:
            result_ = Result::ABORTED;
            reportResult(result_);
            break;
          case Result::CANCELLED:
            result_ = Result::CANCELLED;
            reportResult(result_);
          default:
            // TODO
            break;
        }
      }

      // implement this method in subclasses to handle this tasklet
      virtual Result::Constants process() = 0;

      // override this method in subclasses if necessary
      virtual void reportResult(Result::Constants rc)
      {
        // empty
      }

      virtual std::string toString() const
      {
        std::stringstream ss;
        ss << "Tasklet{result_:" << Result::toString(result_) << "}";
        return ss.str();
      }
    }; /* class Tasklet */

    class Queue
    {
    private:
      mutable pthread_mutex_t lock_;
      std::list<Tasklet *> items_;

    public:
      Queue()
      {
        pthread_mutex_init(&lock_, NULL);
      }

      virtual ~Queue()
      {
        Tasklet * t;
        pthread_mutex_lock(&lock_);
        while (!items_.empty()) {
          t = items_.front();
          items_.pop_front();
          t->reportResult(Tasklet::Result::CANCELLED);
          delete t;
        }
        pthread_mutex_unlock(&lock_);
        pthread_mutex_destroy(&lock_);
      }

      void pushTasklet(Tasklet * t)
      {
        pthread_mutex_lock(&lock_);
        items_.push_back(t);
        pthread_mutex_unlock(&lock_);
      }

      Tasklet * popTasklet()
      {
        Tasklet * t = NULL;
        pthread_mutex_lock(&lock_);
        if (items_.size()) {
          t = items_.front();
          items_.pop_front();
        }
        pthread_mutex_unlock(&lock_);
        return t;
      }

      size_t size() const
      {
        pthread_mutex_lock(&lock_);
        size_t sz = items_.size();
        pthread_mutex_unlock(&lock_);
        return sz;
      }

      bool isEmpty() const
      {
        pthread_mutex_lock(&lock_);
        bool result = items_.empty();
        pthread_mutex_unlock(&lock_);
        return result;
      }
    }; /* class Queue */

    namespace internal
    {
      class ProdConsThread: public Thread
      {
      protected:
        Queue * tasklet_queue_;

      public:
        ProdConsThread(Queue * q = NULL) :
          tasklet_queue_(q)
        {
          attachToQueue(q);
        }

        virtual ~ProdConsThread()
        {

        }

        void attachToQueue(Queue * q)
        {
          tasklet_queue_ = q;
        }

        void detachFromQueue()
        {
          tasklet_queue_ = NULL;
        }
      }; /* class ProdConsThread */
    } /* namespace internal */

    class Consumer: public internal::ProdConsThread
    {
    protected:
      Tasklet * fetchNextTasklet()
      {
        return tasklet_queue_ ? tasklet_queue_->popTasklet() : NULL;
      }

    public:
      Consumer(Queue * q = NULL) :
        ProdConsThread(q)
      {
        attachToQueue(q);
      }

      virtual ~Consumer()
      {

      }

      /*
       * Description:
       *   called before Tasklet `t' is execute()ed
       */
      virtual void prepareBeforeExecute(Tasklet * t)
      {

      }

      /*
       * Description:
       *   called after Tasklet `t' is execute()ed
       */
      virtual void cleanupAfterExecute(Tasklet * t, bool delete_it)
      {
        if (delete_it) {
          delete t;
        }
      }

      /*
       * Description:
       *   Fetch tasklet from the attached queue, then execute.  Execute at most
       *   `limit' tasklets if `limit' is positive, or else fetch/execute until
       *   the queue is empty.
       * Return value:
       *   Number of tasklets fetched and executed.
       */
      size_t fetchTaskletAndExecute(size_t limit = 0)
      {
        size_t count = 0;
        Tasklet * t = NULL;
        while ((limit ? (count < limit) : 1) && (t = fetchNextTasklet())) {
          prepareBeforeExecute(t);
          t->execute();
          ++count;
          cleanupAfterExecute(t, true);
        }
        return count;
      }

      // override this method in subclasses if necessary
      virtual void * routine()
      {
        while (getStopFlag()) {
          size_t executed UNUSED = fetchTaskletAndExecute();
        }
        return NULL;
      }
    }; /* class Consumer */

    class Producer: public internal::ProdConsThread
    {
    protected:
      bool dispatchTasklet(Tasklet * t)
      {
        if (!tasklet_queue_) {
          return false;
        }
        tasklet_queue_->pushTasklet(t);
        return true;
      }

    public:
      Producer(Queue * q) :
        ProdConsThread(q)
      {

      }

      virtual ~Producer()
      {

      }

      /*
       * Description:
       *   Called before `t' is pushed into the queue.
       */
      virtual void prepareBeforeDispatch(Tasklet * t)
      {

      }

      /*
       * Description:
       *   Called if failed pushing `t' into the queue.
       */
      virtual void cleanupAfterDispatchFailure(Tasklet * t, bool delete_tasklet)
      {
        if (delete_tasklet) {
          delete t;
        }
      }

      /*
       * Description:
       *   Generate tasklet and push into the attached queue.  Generate at most
       *   `limit' tasklet if `limit' is positive, or else until not more
       *   tasklet is available.
       * Return value:
       *   Number of tasklets generated, including those not pushed into the
       *   attached queue.
       */
      size_t generateTaskletAndDispatch(size_t limit = 0)
      {
        size_t count = 0;
        Tasklet * t = NULL;
        while ((limit ? (count < limit) : 1) && (t = generateNextTasklet())) {
          ++count;
          prepareBeforeDispatch(t);
          if (!dispatchTasklet(t)) {
            cleanupAfterDispatchFailure(t, true);
          }
        }
        return count;
      }

      virtual Tasklet * generateNextTasklet() = 0;

      // override this method in subclasses if necessary
      virtual void * routine()
      {
        while (!getStopFlag()) {
          size_t generated UNUSED = generateTaskletAndDispatch();
        }
        return NULL;
      }
    }; /* class Producer */

  } /* namespace ProducerAndConsumer */
}

#endif /* _BrianZ_NEBULA_PRODUCER_CONSUMER_H_ */
