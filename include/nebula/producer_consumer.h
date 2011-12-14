/*
 * producer_consumer.h
 *
 *  Created on: Dec 14, 2011
 *  Author:     Brian Y. ZHANG
 *  Email:      brianzhang at gmail dot com
 */

#ifndef _BrianZ_NEBULA_PRODUCER_CONSUMER_H_
#define _BrianZ_NEBULA_PRODUCER_CONSUMER_H_

#include <pthread.h>
#include <list>
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
          NOT_AVAILABLE = 0, OK = 1, FAILED = 2, ABORTED = 3
        };

        const char * toString(Constants v)
        {
          switch (v) {
            case NOT_AVAILABLE:
              return "NOT_AVAILABLE";
              break;
            case OK:
              return "OK";
              break;
            case FAILED:
              return "FAILED";
              break;
            case ABORTED:
              return "ABORTED";
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

      virtual Result::Constants process() = 0;

      void execute()
      {
        switch (process()) {
          case Result::OK:
            result_ = Result::OK;
            reportSuccess();
            break;
          case Result::FAILED:
            result_ = Result::FAILED;
            reportFailure();
            break;
          case Result::ABORTED:
            result_ = Result::ABORTED;
            reportAbortion();
            break;
          default:
            // TODO
            break;
        }
      }

      // override following method if necessary
      virtual void reportSuccess()
      {
        // empty
      }

      virtual void reportFailure()
      {
        // empty
      }

      virtual void reportAbortion()
      {
        // empty
      }

      virtual std::string toString() const
      {
        std::stringstream ss;
        ss << "Tasklet{}";
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

    class Consumer: public Thread
    {
    private:
      Queue * tasklet_queue_;

    public:
      Consumer(Queue * q = NULL) :
        tasklet_queue_(NULL)
      {
        attachToQueue(q);
      }

      virtual ~Consumer()
      {

      }

      void attachToQueue(Queue * q)
      {
        tasklet_queue_ = q;
      }

      void detathFromQueue()
      {
        tasklet_queue_ = NULL;
      }

    protected:
      Tasklet * fetchOneTasklet()
      {
        return tasklet_queue_ ? tasklet_queue_->popTasklet() : NULL;
      }

      virtual void prepareBeforeExecute(Tasklet * t)
      {

      }

      virtual void cleanupAfterExecute(Tasklet * t)
      {

      }

      void executeUntilQueueIsEmpty()
      {
        Tasklet * t = NULL;
        while (tasklet_queue_ && (t = tasklet_queue_->popTasklet())) {
          prepareBeforeExecute(t);
          t->execute();
          cleanupAfterExecute(t);
          delete t;
        }
      }
    };

  } /* namespace ProducerAndConsumer */
}

#endif /* _BrianZ_NEBULA_PRODUCER_CONSUMER_H_ */
