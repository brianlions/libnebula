/*
 * async_io_t.cc
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
#include "nebula/async_io.h"

using nebula::AsyncIo;

class AsyncIoTS: public testing::Test
{
protected:
  int pipefds_[2];
  const char * name_template_;
  int open_fd_;
  char file_name_[64];

  virtual void SetUp()
  {
    pipefds_[0] = pipefds_[1] = -1;
    name_template_ = "ts_async_funcs_XXXXXX";
    open_fd_ = -1;
    strncpy(file_name_, name_template_, sizeof(file_name_));
    EXPECT_TRUE((open_fd_ = mkstemp(file_name_)) >= 0);
  }

  virtual void TearDown()
  {
    if (pipefds_[0] >= 0) {
      close(pipefds_[0]);
    }
    if (pipefds_[1] >= 0) {
      close(pipefds_[1]);
    }
    if (open_fd_ >= 0) {
      close(open_fd_);
      unlink(file_name_);
    }
  }
};

TEST_F(AsyncIoTS, nonblock)
{
  EXPECT_TRUE(AsyncIo::isNonblock_3state(open_fd_) == 0);
  EXPECT_TRUE(AsyncIo::setNonblock(open_fd_) == 0);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(open_fd_) == 1);
}

TEST_F(AsyncIoTS, closeOnExec)
{
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(open_fd_) == 0);
  EXPECT_TRUE(AsyncIo::setCloseOnExec(open_fd_) == 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(open_fd_) == 1);
}

TEST_F(AsyncIoTS, pipe2_none)
{
  EXPECT_TRUE(AsyncIo::pipe2(pipefds_, 0) == 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[0])== 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[1]) == 0);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[0]) == 0);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[1]) == 0);
}

TEST_F(AsyncIoTS, pipe2_closeOnExec)
{
  EXPECT_TRUE(AsyncIo::pipe2(pipefds_, AsyncIo::AIO_CLOEXEC) == 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[0]) == 1);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[1]) == 1);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[0]) == 0);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[1]) == 0);
}

TEST_F(AsyncIoTS, pipe2_nonblock)
{
  EXPECT_TRUE(AsyncIo::pipe2(pipefds_, AsyncIo::AIO_NONBLOCK) == 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[0]) == 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[1]) == 0);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[0]) == 1);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[1]) == 1);
}

TEST_F(AsyncIoTS, pipe2_closeOnExec_nonblock)
{
  EXPECT_TRUE(AsyncIo::pipe2(pipefds_, AsyncIo::AIO_NONBLOCK | AsyncIo::AIO_CLOEXEC) == 0);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[0]) == 1);
  EXPECT_TRUE(AsyncIo::isCloseOnExec_3state(pipefds_[1]) == 1);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[0]) == 1);
  EXPECT_TRUE(AsyncIo::isNonblock_3state(pipefds_[1]) == 1);
}
