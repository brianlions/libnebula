/*
 * producer_consumer_t.cc
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

#include <vector>
#include <iostream>
#include <gtest/gtest.h>
#include "nebula/producer_consumer.h"
#include "nebula/time.h"
#include "nebula/pretty_message.h"

namespace MyAliasProdCons = nebula::ProducerAndConsumer;

class MyTasklet: public MyAliasProdCons::Tasklet
{
private:
  int orig_;
  int var_;
  volatile int * sum_;
public:
  MyTasklet(int v = 0, volatile int * sum = NULL) :
    orig_(0), var_(v), sum_(sum)
  {
  }

  virtual Result::Constants process()
  {
    if (sum_) {
      orig_ = __sync_fetch_and_add(sum_, var_);
      return Result::SUCCESS;
    }
    else {
      return Result::ABORTED;
    }
  }

  virtual std::string toString() const
  {
    std::stringstream ss;
    ss << "MyTasklet{var_:" << var_ << ",orig_:" << orig_ << ",sum_:" << ((void *) sum_) << "," << Tasklet::toString()
        << "}";
    return ss.str();
  }
}; /* class MyTasklet */

static const int MAX_VALUE = 100;
static const int TOTAL_NUM_OF_WORKERS = 5;
static const unsigned int LOOP_INTERVAL_MS = 100;
static const unsigned int TIMECOST_OF_PRODUCE = 50;
static volatile int sum = 0;
static volatile int num_of_active_producers = 0;

static int getNumOfActiveProducers()
{
  return __sync_fetch_and_add(&num_of_active_producers, 0);
}

static int incrNumOfActiveProducers()
{
  return __sync_add_and_fetch(&num_of_active_producers, 1);
}

static int decrNumOfActiveProducers()
{
  return __sync_sub_and_fetch(&num_of_active_producers, 1);
}

class MyProducer: public MyAliasProdCons::Producer
{
private:
  size_t limit_;
  size_t count_;
public:
  MyProducer(MyAliasProdCons::Queue * q = NULL, size_t limit = 0) :
    Producer(q), limit_(limit), count_(0)
  {
  }
  virtual MyAliasProdCons::Tasklet * generateNextTasklet()
  {
    if (count_ < limit_) {
      nebula::Time::msSleep(TIMECOST_OF_PRODUCE);
      count_++;
      DEV_MESSAGE("takelet id: %lu", count_);
      return new MyTasklet(count_, &sum);
    }
    else {
      return NULL;
    }
  }
  virtual void * routine()
  {
    incrNumOfActiveProducers();
    while (!getStopFlag()) {
      generateTaskletAndDispatch();
      if (count_ == limit_) {
        break;
      }
      nebula::Time::msSleep(LOOP_INTERVAL_MS);
    }
    decrNumOfActiveProducers();
    return NULL;
  }
};

class MyConsumer: public MyAliasProdCons::Consumer
{
public:
  MyConsumer(MyAliasProdCons::Queue * q = NULL) :
    Consumer(q)
  {
  }
  virtual void prepareBeforeExecute(MyAliasProdCons::Tasklet * t)
  {
    DEV_MESSAGE("%s", t->toString().c_str());
  }
  virtual void cleanupAfterExecute(MyAliasProdCons::Tasklet * t, bool delete_it)
  {
    DEV_MESSAGE("%s", t->toString().c_str());
    MyAliasProdCons::Consumer::cleanupAfterExecute(t, delete_it);
  }
  virtual void * routine()
  {
    DEV_MESSAGE("consumer 0x%016lu started ...", self());
    while (!getStopFlag()) {
      fetchTaskletAndExecute();
      if (!getNumOfActiveProducers()) {
        break;
      }
      nebula::Time::msSleep(LOOP_INTERVAL_MS);
    }
    return NULL;
  }
};

TEST(ProducerAndConsumerTS, casePoc)
{
  MyAliasProdCons::Queue shared_queue;

  std::cout << "this test case will finish in " << (MAX_VALUE * TIMECOST_OF_PRODUCE) << " msec" << std::endl;

  MyProducer producer(&shared_queue, MAX_VALUE);
  producer.create();

  std::vector<MyConsumer *> consumers;
  for (size_t i = 0; i < (size_t) TOTAL_NUM_OF_WORKERS; ++i) {
    consumers.push_back(new MyConsumer(&shared_queue));
  }
  for (size_t i = 0; i < (size_t) TOTAL_NUM_OF_WORKERS; ++i) {
    consumers[i]->create();
  }

  DEV_MESSAGE("waiting for the producer to terminate ...");
  producer.join();
  DEV_MESSAGE("producer terminated, waiting for consumsers to terminate ...");
  for (size_t i = 0; i < consumers.size(); ++i) {
    consumers[i]->join();
    DEV_MESSAGE("consumer %lu of %lu terminated.", (i + 1), consumers.size());
    delete consumers[i];
  }

  EXPECT_EQ(sum, (1 + MAX_VALUE) * MAX_VALUE / 2);
}
