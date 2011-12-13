/*
 * async_event_t.cc
 *
 *  Created on: Dec 10, 2011
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

#include <gtest/gtest.h>
#include "nebula/thread.h"
#include "nebula/async_event.h"
#include "nebula/async_io.h"

#if 0
#define FPRINTF_FLUSH(stream, fmt, ...) \
do { \
    fprintf((stream), "[%s:%d:%s] " fmt, __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
    fflush((stream)); \
} while (0)
#endif

using nebula::AsyncEvent;
using nebula::AsyncIo;

class AsyncEventTS: public testing::Test
{
protected:
  AsyncEvent * ae;
  int epoll_hint_size;
  int max_time_events;

  virtual void SetUp()
  {
    ae = new AsyncEvent();
    epoll_hint_size = 32;
    max_time_events = 96;

    EXPECT_EQ(ae->initialize(epoll_hint_size, max_time_events), 0);
    EXPECT_EQ(ae->fdTableSize(), getdtablesize());
    EXPECT_EQ(ae->numMonitoredFds(), 0);
    EXPECT_EQ(ae->numReadyFds(), 0);
    EXPECT_EQ(ae->timeEventTableSize(), max_time_events);
    EXPECT_EQ(ae->numTimeEvents(), 0);
  }

  virtual void TearDown()
  {
    if (ae) {
      EXPECT_EQ(0, ae->numMonitoredFds());
      EXPECT_EQ(0, ae->numReadyFds());
      EXPECT_EQ(0, ae->numTimeEvents());
      delete ae;
    }
  }
};

//---------------------------------------------------------------------------------------------------------------------

uint32_t oneShotTmEvtCb(uint64_t time_event_id, void * data, AsyncEvent * ae)
{
  uint64_t * expect_id = (uint64_t *) data;
  EXPECT_EQ(*expect_id, time_event_id);
  EXPECT_NE(*expect_id, time_event_id + 1);
  EXPECT_EQ(ae->numTimeEvents(), 1);
  EXPECT_NE(ae->numTimeEvents(), 2);
  ae->stop();
  return 0;
}

TEST_F(AsyncEventTS, caseOneShotTimeEvent)
{
  int32_t ms_left = 300;
  uint64_t time_event_id;
  DEV_MESSAGE("this test should finish in %d msec ...", ms_left);
  EXPECT_EQ(0, ae->addTimeEvent(ms_left, oneShotTmEvtCb, &time_event_id, &time_event_id));
  ae->eventLoop();
  EXPECT_EQ((uint64_t) 1, ae->lastTimeEventId());
  DEV_MESSAGE("test finished");
}

//---------------------------------------------------------------------------------------------------------------------

struct TimeEventSpecfication
{
  uint32_t interval_ms;
  uint32_t num_iterations;
  uint32_t max_iterations;
  uint64_t time_event_id;
};

uint32_t multiShotTmEvtCb(uint64_t time_event_id, void * data, AsyncEvent * ae)
{
  struct TimeEventSpecfication * spec = (struct TimeEventSpecfication *) data;
  DEV_MESSAGE("interval_ms=%u,num_iterations=%u,max_iterations=%u,time_event_id=%lu", spec->interval_ms,
      spec->num_iterations, spec->max_iterations, spec->time_event_id);
  EXPECT_EQ(spec->time_event_id, time_event_id);
  EXPECT_TRUE(spec->num_iterations < spec->max_iterations);
  //    ++(spec->num_iterations);
  if (++(spec->num_iterations) < spec->max_iterations) {
    //        DEV_MESSAGE("returning ...");
    return spec->interval_ms;
  }
  else {
    //        DEV_MESSAGE("stopping ...");
    ae->stop();
    return 0;
  }
}

TEST_F(AsyncEventTS, caseMultiShotTimeEvent)
{
  struct TimeEventSpecfication spec;
  spec.interval_ms = 500;
  spec.num_iterations = 0;
  spec.max_iterations = 5;
  spec.time_event_id = 0;
  DEV_MESSAGE("this test should finish in %d msec ...", spec.interval_ms * spec.max_iterations);
  EXPECT_EQ(0, ae->addTimeEvent(spec.interval_ms, multiShotTmEvtCb, &spec, &(spec.time_event_id)));
  ae->eventLoop();
  EXPECT_EQ((uint64_t) 1, ae->lastTimeEventId());
  EXPECT_EQ(spec.num_iterations, spec.max_iterations);
  DEV_MESSAGE("test finished");
}

//---------------------------------------------------------------------------------------------------------------------

struct TimeEventSpecExt: public TimeEventSpecfication
{
  uint32_t slowest;
};

uint32_t multiTmEvtCb(uint64_t time_event_id, void * data, AsyncEvent * ae)
{
  struct TimeEventSpecExt * spec = (struct TimeEventSpecExt *) data;
  DEV_MESSAGE("interval_ms=%u,num_iterations=%u,max_iterations=%u,time_event_id=%lu", spec->interval_ms,
      spec->num_iterations, spec->max_iterations, spec->time_event_id);
  EXPECT_EQ(spec->time_event_id, time_event_id);
  EXPECT_TRUE(spec->num_iterations < spec->max_iterations);
  //    ++(spec->num_iterations);
  if (++(spec->num_iterations) < spec->max_iterations) {
    //        DEV_MESSAGE("returning ...");
    return spec->interval_ms;
  }
  else {
    if (spec->slowest) {
      //            DEV_MESSAGE("stopping ...");
      ae->stop();
    }
    return 0;
  }
}

TEST_F(AsyncEventTS, caseMultiTimeEvents)
{
  const uint32_t num_specs = 30;
  struct TimeEventSpecExt specs[num_specs];
  uint32_t time_span_ms = 0;
  for (uint32_t i = 0; i < num_specs; ++i) {
    specs[i].interval_ms = 3 * (i + 5);
    specs[i].num_iterations = 0;
    specs[i].max_iterations = i + 3;
    specs[i].time_event_id = 0;

    if (i == num_specs - 1) {
      time_span_ms = specs[i].interval_ms * specs[i].max_iterations;
      specs[i].slowest = 1;
    }
    else {
      specs[i].slowest = 0;
    }
  }

  DEV_MESSAGE("this test should finish in %d msec ...", time_span_ms);

  for (uint32_t i = 0; i < num_specs; ++i) {
    EXPECT_EQ(0, ae->addTimeEvent(specs[i].interval_ms, multiTmEvtCb, specs + i, &((specs + i)->time_event_id)));
    EXPECT_EQ((uint64_t) (i + 1), ae->lastTimeEventId());
    EXPECT_EQ((int) (i + 1), ae->numTimeEvents());
  }
  ae->eventLoop();

  for (uint32_t i = 0; i < num_specs; ++i) {
    EXPECT_EQ(specs[i].num_iterations, specs[i].max_iterations);
  }

  DEV_MESSAGE("test finished");
}

//---------------------------------------------------------------------------------------------------------------------

struct SockIoInfo
{
  int sockfd_;
  size_t nread_;
  size_t nwrite_;
};

/*
 * Description:
 *   For every socket fd added through addFileDescriptor(), writes `min_write + sockfd' bytes to that socket, and exit
 *   if there's no I/O operation in `check_interval_ms * max_check' milliseconds.
 */
class AeServer: public nebula::Thread
{
private:
  struct ConnectionIoInfo
  {
    size_t nread_;
    size_t nwrite_;

    ConnectionIoInfo() :
      nread_(0), nwrite_(0)
    {

    }
  };

public:

  AeServer() :
    Thread(), io_info_(NULL), check_interval_ms_(0), managed_fds_(NULL), max_total_fds_(0), num_used_fds_(0),
        min_write_(0), max_check_(0), num_check_(0)
  {

  }

  ~AeServer()
  {
    if (io_info_) {
      free(io_info_);
    }

    if (managed_fds_) {
      free(managed_fds_);
    }
  }

  int initialize(size_t min_write, int max_total_fds = 32, uint32_t check_interval_ms = 1000, uint32_t max_check = 10)
  {
    if (async_event_.initialize(max_total_fds) < 0) {
      return -1;
    }

    if (!(io_info_ = (struct ConnectionIoInfo *) calloc(async_event_.fdTableSize(), sizeof(*io_info_)))) {
      return -1;
    }

    check_interval_ms_ = check_interval_ms;

    if (!(managed_fds_ = (int *) malloc(max_total_fds * sizeof(managed_fds_[0])))) {
      return -1;
    }
    max_total_fds_ = max_total_fds;
    num_used_fds_ = 0;

    min_write_ = min_write;
    max_check_ = max_check;

    return 0;
  }

  int addFileDescriptor(int fd)
  {
    if (num_used_fds_ < max_total_fds_) {
      managed_fds_[num_used_fds_++] = fd;
      return 0;
    }

    return -1;
  }

  virtual void * routine()
  {
    DEV_MESSAGE("thread is starting ...");

    for (int i = 0; i < num_used_fds_; ++i) {
      if (AsyncIo::setNonblock(managed_fds_[i]) < 0) {
        DEV_MESSAGE("AsyncIo::setNonblock(%d) failed", managed_fds_[i]);
        continue;
      }
      EXPECT_EQ(0, async_event_.addFdEvent(managed_fds_[i], AsyncEvent::RDWR
              , sockIoHandler, this
              , sockErrorHandler, this
          ));
    }

    uint64_t time_event_id;
    EXPECT_EQ(0, async_event_.addTimeEvent(check_interval_ms_, periodicalCheck, this, &time_event_id));

    while (!getStopFlag() && (async_event_.numMonitoredFds() > 0 || async_event_.numTimeEvents() > 0)) {
      async_event_.eventLoop();
    }

    struct SockIoInfo * result = (struct SockIoInfo *) calloc((size_t) num_used_fds_, sizeof(*result));
    if (result) {
      for (int i = 0; i < num_used_fds_; ++i) {
        int fd = managed_fds_[i];
        result[i].sockfd_ = fd;
        result[i].nread_ = io_info_[fd].nread_;
        result[i].nwrite_ = io_info_[fd].nwrite_;
      }
    }

    DEV_MESSAGE("thread is exiting ...");

    return result;
  }

  static uint32_t sockIoHandler(int fd, uint32_t mask, void * data, AsyncEvent * ae)
  {
    AeServer * me = (AeServer *) data;

    struct ConnectionIoInfo * conn_info = me->io_info_ + fd;

    char buf[1024];
    memset(buf, 's', sizeof(buf));
    ssize_t num;

    if (mask & AsyncEvent::READABLE) {
      recv_data:
      while ((num = recv(fd, buf, sizeof(buf), 0)) > 0) {
        conn_info->nread_ += num;
      }
      me->num_check_ = 0; // reset it
      if (num == 0) {
        EXPECT_EQ(0, me->async_event_.removeFd(fd, true));
      }
      else if (errno == EINTR) {
        goto recv_data;
      }
      else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // nothing to do
      }
      else {
        DEV_MESSAGE("recv() failed: fd=%d,errno=%d", fd, errno);
      }
    }

    if (mask & AsyncEvent::WRITABLE) {
      size_t left;
      send_data:
      while ((left = me->min_write_ + fd - conn_info->nwrite_) > 0
          && (num = send(fd, buf, left < sizeof(buf) ? left : sizeof(buf), MSG_NOSIGNAL)) > 0) {
        conn_info->nwrite_ += num;
      }
      me->num_check_ = 0; // reset it
      if (left == 0) {
        EXPECT_EQ(0, me->async_event_.delFdEvent(fd, AsyncEvent::WRITABLE));
      }
      else if (errno == EINTR) {
        goto send_data;
      }
      else if (errno == EAGAIN || errno == EWOULDBLOCK) {

      }
      else {
        DEV_MESSAGE("send() failed: fd=%d,errno=%d", fd, errno);
      }
    }

    return AsyncEvent::NONE;
  }

  static uint32_t sockErrorHandler(int fd, uint32_t mask_unused, void * data, AsyncEvent * ae)
  {
    AeServer * me = (AeServer *) data;

    DEV_MESSAGE("server - removing fd=%d because of exception", fd);
    EXPECT_EQ(0, me->async_event_.removeFd(fd, true));

    return mask_unused;
  }

  static uint32_t periodicalCheck(uint64_t time_event_id, void * data, AsyncEvent * ae)
  {
    AeServer * me = (AeServer *) data;

    if (me->getStopFlag()) {
      return 0;
    }

    if (++(me->num_check_) >= me->max_check_) {
      DEV_MESSAGE("server - no i/o in last %u * %u ms, stopping ...", me->check_interval_ms_, me->max_check_);
      me->async_event_.stop();
      me->setStopFlag();
      return 0;
    }

    return me->check_interval_ms_;
  }

private:
  AsyncEvent async_event_;
  struct ConnectionIoInfo * io_info_;

  uint32_t check_interval_ms_;

  int * managed_fds_;
  int max_total_fds_;
  int num_used_fds_;

  size_t min_write_;
  uint32_t max_check_;
  uint32_t num_check_;
};

/*
 * Description:
 *   Thread writes `min_write + sockfd' bytes to `sockfd', and will exit if no I/O operations in
 *   `check_interval_ms * max_check' milliseconds.
 */
class AeClient: public nebula::Thread
{
public:
  AeClient() :
    Thread(), min_write_(0), sockfd_(-1), nread_(0), nwrite_(0), check_interval_ms_(0), max_check_(0), num_check_(0)
  {

  }

  ~AeClient()
  {

  }

  int initialize(size_t min_write, int sockfd, uint32_t check_interval_ms = 1000, uint32_t max_check = 10)
  {
    if (async_event_.initialize() < 0) {
      return -1;
    }

    min_write_ = min_write;
    sockfd_ = sockfd;
    check_interval_ms_ = check_interval_ms;
    max_check_ = max_check;

    return 0;
  }

  virtual void * routine()
  {
    DEV_MESSAGE("thread is starting ...");

    EXPECT_EQ(0, AsyncIo::setNonblock(sockfd_));
    EXPECT_EQ(0, async_event_.addFdEvent(sockfd_, AsyncEvent::RDWR
            , sockIoHandler, this
            , sockErrorHandler, this
        ));

    uint64_t time_event_id;
    EXPECT_EQ(0, async_event_.addTimeEvent(check_interval_ms_, periodicalCheck, this, &time_event_id));

    while (!getStopFlag() && (async_event_.numMonitoredFds() > 0 || async_event_.numTimeEvents() > 0)) {
      async_event_.eventLoop();
    }

    // todo prepare return value
    struct SockIoInfo * result = (struct SockIoInfo *) malloc(sizeof(*result));
    if (result) {
      result->sockfd_ = sockfd_;
      result->nread_ = nread_;
      result->nwrite_ = nwrite_;
    }

    DEV_MESSAGE("thread is exiting ...");

    return result;
  }

  static uint32_t sockIoHandler(int fd, uint32_t mask, void * data, AsyncEvent * ae)
  {
    AeClient * me = (AeClient *) data;

    char buf[1024];
    memset(buf, 'c', sizeof(buf));
    ssize_t num;

    if (mask & AsyncEvent::READABLE) {
      recv_data:
      while ((num = recv(fd, buf, sizeof(buf), 0)) > 0) {
        me->nread_ += num;
      }
      me->num_check_ = 0; // reset it
      if (num == 0) {
        EXPECT_EQ(0, me->async_event_.removeFd(fd, true));
      }
      else if (errno == EINTR) {
        goto recv_data;
      }
      else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // nothing to do
      }
      else {
        DEV_MESSAGE("recv() failed: fd=%d,errno=%d", fd, errno);
      }
    }

    if (mask & AsyncEvent::WRITABLE) {
      size_t left;
      send_data:
      while ((left = me->min_write_ + fd - me->nwrite_) > 0
          && (num = send(fd, buf, left < sizeof(buf) ? left : sizeof(buf), MSG_NOSIGNAL)) > 0) {
        me->nwrite_ += num;
      }
      me->num_check_ = 0; // reset it
      if (left == 0) {
        EXPECT_EQ(0, me->async_event_.delFdEvent(fd, AsyncEvent::WRITABLE));
      }
      else if (errno == EINTR) {
        goto send_data;
      }
      else if (errno == EAGAIN || errno == EWOULDBLOCK) {

      }
      else {
        DEV_MESSAGE("send() failed: fd=%d,errno=%d", fd, errno);
      }
    }

    return AsyncEvent::NONE;
  }

  static uint32_t sockErrorHandler(int fd, uint32_t mask_unused, void * data, AsyncEvent * ae)
  {
    AeClient * me = (AeClient *) data;

    DEV_MESSAGE("client - removing fd=%d because of exception", fd);
    EXPECT_EQ(0, me->async_event_.removeFd(fd, true));

    return mask_unused;
  }

  static uint32_t periodicalCheck(uint64_t time_event_id, void * data, AsyncEvent * ae)
  {
    AeClient * me = (AeClient *) data;

    DEV_MESSAGE("client internal stat: idStopping()=%d,min_write_=%lu,sockfd_=%d,nread_=%lu,nwrite_=%lu,"
        "check_interval_ms_=%u,max_check_=%u,num_check_=%u",
        me->isStopping(), me->min_write_, me->sockfd_, me->nread_, me->nwrite_,
        me->check_interval_ms_, me->max_check_, me->num_check_);

    if (me->getStopFlag()) {
      return 0;
    }

    if (++(me->num_check_) >= me->max_check_) {
      DEV_MESSAGE("client - no i/o in last %u * %u ms, stopping ...", me->check_interval_ms_, me->max_check_);
      me->async_event_.stop();
      me->setStopFlag();
      return 0;
    }

    return me->check_interval_ms_;
  }

private:
  AsyncEvent async_event_;

  size_t min_write_;
  int sockfd_;
  size_t nread_;
  size_t nwrite_;

  uint32_t check_interval_ms_;
  uint32_t max_check_;
  uint32_t num_check_;
};

TEST_F(AsyncEventTS, caseServerOnly)
{
  const int32_t max_total_clients = 20;
  const size_t min_write = 1024 * 1024;
  const uint32_t check_interval_ms = 1000;
  const uint32_t max_check = 5;

  struct SockIoInfo * result;
  AeServer server;
  EXPECT_EQ(0, server.initialize(min_write, max_total_clients, check_interval_ms, max_check));
  EXPECT_EQ(0, server.create());
#if 1
  // either one is ok
  DEV_MESSAGE("this test should finish in %u msec ...", check_interval_ms * max_check);
  server.join((void **) &result);
  EXPECT_TRUE(result != NULL);
  free(result);
  DEV_MESSAGE("test finished.");
#else
  const unsigned int pause_time = 5;
  DEV_MESSAGE("this test should finish in %d sec ...", pause_time);
  sleep(pause_time);
  server.stop();
  server.join(NULL);
  DEV_MESSAGE("test finished.");
#endif
}

TEST_F(AsyncEventTS, caseClientOnly)
{
  const size_t min_write = 1024 * 1024;
  const uint32_t check_interval_ms = 1000;
  const uint32_t max_check = 5;

  int pair[2];
  EXPECT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, pair));
  DEV_MESSAGE("af_unix stream socket pair (%d,%d)", pair[0], pair[1]);

  struct SockIoInfo * result;
  AeClient client;
  EXPECT_EQ(0, client.initialize(min_write, pair[0], check_interval_ms, max_check));
  EXPECT_EQ(0, client.create());
  client.join((void **) &result);
  EXPECT_TRUE(result != NULL);
  free(result);
}

TEST_F(AsyncEventTS, caseClientServer)
{
  const size_t min_write = 1000000;
  const uint32_t check_interval_ms = 1000;
  const uint32_t max_check = 5;

  const uint32_t num_clients = 10;

  struct IoCatalog
  {
    int pair[2];
    size_t serv_read;
    size_t serv_write;
    size_t clnt_read;
    size_t clnt_write;

    IoCatalog() :
      serv_read(0), serv_write(0), clnt_read(0), clnt_write(0)
    {
      pair[0] = pair[1] = -1;
    }
  };

  struct IoCatalog catalog[num_clients];

  AeServer server;
  AeClient clients[num_clients];
  ASSERT_EQ(0, server.initialize(min_write, num_clients, check_interval_ms, max_check));
  for (uint32_t i = 0; i < num_clients; ++i) {
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, catalog[i].pair));
    ASSERT_EQ(0, server.addFileDescriptor(catalog[i].pair[0]));
    ASSERT_EQ(0, clients[i].initialize(min_write, catalog[i].pair[1], check_interval_ms, max_check));
  }

  ASSERT_EQ(0, server.create());
  for (uint32_t i = 0; i < num_clients; ++i) {
    ASSERT_EQ(0, clients[i].create());
  }
  struct SockIoInfo * server_return = NULL;
  EXPECT_EQ(0, server.join((void **) &server_return));
  EXPECT_TRUE(server_return != NULL);
  for (uint32_t i = 0; i < num_clients; ++i) {
    int serv_fd = server_return[i].sockfd_;
    for (uint32_t j = 0; j < num_clients; ++j) {
      if (catalog[j].pair[0] == serv_fd) {
        catalog[j].serv_read = server_return[i].nread_;
        catalog[j].serv_write = server_return[i].nwrite_;
      }
    }
  }
  free(server_return);

  for (uint32_t i = 0; i < num_clients; ++i) {
    struct SockIoInfo * client_return = NULL;
    EXPECT_EQ(0, clients[i].join((void **) &client_return));
    EXPECT_TRUE(client_return != NULL);
    for (uint32_t j = 0; j < num_clients; ++j) {
      if (catalog[j].pair[1] == client_return->sockfd_) {
        catalog[j].clnt_read = client_return->nread_;
        catalog[j].clnt_write = client_return->nwrite_;
      }
    }
    free(client_return);
  }

  for (uint32_t i = 0; i < num_clients; ++i) {
    EXPECT_EQ(catalog[i].serv_read, catalog[i].clnt_write);
    EXPECT_EQ(catalog[i].serv_write, catalog[i].clnt_read);
    EXPECT_EQ(catalog[i].serv_write, min_write + catalog[i].pair[0]);
    EXPECT_EQ(catalog[i].clnt_write, min_write + catalog[i].pair[1]);
    DEV_MESSAGE("%u: serv_fd:%d, clnt_fd:%d, serv_rd:%lu, clnt_wr:%lu, serv_wr:%lu, clnt_rd:%lu",
        i, catalog[i].pair[0], catalog[i].pair[1],
        catalog[i].serv_read, catalog[i].clnt_write, catalog[i].serv_write, catalog[i].clnt_read);
  }
}
