/*
 * app_logger_t.cc
 *
 *  Created on: Dec 17, 2011
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

//#ifndef USE_PRETTY_MESSAGE
//#define USE_PRETTY_MESSAGE
//#endif
#include <stdint.h>
#include <vector>
#include <iostream>
#include <gtest/gtest.h>
#include "nebula/pretty_message.h"
#include "nebula/app_logger.h"
#include "nebula/thread.h"
#include "nebula/time.h"

namespace lm = nebula::LogModule;

class AppLoggerTS: public testing::Test
{
protected:

  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

TEST_F(AppLoggerTS, caseDiskFile)
{
  lm::internal::FileHandle fh;
  fh.setConfig(".", "foo", 5);
  char buf[100];
  int len;
  for (int i = 0; i < 23; ++i) {
    len = snprintf(buf, sizeof(buf), "message line %d\n", i + 1);
    fh.writeMessage(buf, len);
    if (i % 10 == 9) {
      fh.switchFile();
    }
  }
}

TEST_F(AppLoggerTS, caseDevTty)
{
  lm::internal::FileHandle fh;
  fh.setConfig();
  char buf[100];
  int len;
  for (int i = 0; i < 20; ++i) {
    len = snprintf(buf, sizeof(buf), "message line %d\n", i + 1);
    fh.writeMessage(buf, len);
    if (i % 10 == 9) {
      fh.switchFile();
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------

static const uint32_t num_messages = 100;
static const uint32_t wait_interval_ms = 10;
static const uint32_t num_workers = 10;

class AppLoggerTS2: public testing::Test
{
protected:
  //  lm::LogServer * uniq_instance_;

  virtual void SetUp()
  {
    lm::startLogServer(".", "testsuite_app_logger_2", 1);
  }

  virtual void TearDown()
  {
    lm::stopLogServer();
  }
};

class MessageGenerator: public nebula::Thread
{
private:
  int my_id_;
  lm::LogLevel::Constants log_level_;
  lm::LogHandle * log_handle_;

public:
  MessageGenerator(int worker_id, lm::LogLevel::Constants level) :
    my_id_(worker_id), log_level_(level), log_handle_(NULL)
  {

  }

  ~MessageGenerator()
  {
    if (log_handle_) {
      delete log_handle_;
    }
  }

  virtual void * routine()
  {
    char identity[64];
    snprintf(identity, sizeof(identity), "worker%02d", my_id_);
    DEV_MESSAGE("identity: `%s'", identity);
    if (!(log_handle_ = lm::getLogHandle(identity, log_level_))) {
      return (void *) errno;
    }
    for (uint32_t i = 0; i < num_messages; ++i) {
      APPLOG_DEBUG(log_handle_, "message %u of %u", (i + 1), num_messages);
      APPLOG_INFO(log_handle_, "message %u of %u", (i + 1), num_messages);
      APPLOG_NOTICE(log_handle_, "message %u of %u", (i + 1), num_messages);
      nebula::Time::msSleep(wait_interval_ms);
    }
    return (void *) 0;
  }
};

TEST_F(AppLoggerTS2, caseLogServerAndClients)
{
  std::vector<MessageGenerator *> workers;
  for (uint32_t i = 0; i < num_workers; ++i) {
    MessageGenerator * worker = new MessageGenerator(i + 1, (i % 2 == 0) ? lm::LogLevel::DEBUG : lm::LogLevel::INFO);
    worker->create();
    workers.push_back(worker);
  }
  for (size_t i = 0; i < workers.size(); ++i) {
    workers[i]->join();
    std::cout << "worker " << (i + 1) << " of " << workers.size() << " terminated." << std::endl;
    delete workers[i];
  }
}
