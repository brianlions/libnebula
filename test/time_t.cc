/*
 * time_t.cc
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

#include <iostream>
#include <gtest/gtest.h>
#include "nebula/time.h"

using nebula::Time;
using nebula::StopWatch;

class TSTime: public testing::Test
{
protected:

  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

TEST_F(TSTime, caseFormatDataTime)
{
  time_t t = 0;
  char buf[128];
  std::cout << "epoch: " << Time::formatDataTime(buf, sizeof(buf), "%Y.%m.%d.%H.%M.%S.XXXXXX", &t) << std::endl;
  std::cout << "now:   " << Time::formatDataTime(buf, sizeof(buf), "%Y.%m.%d.%H.%M.%S.XXXXXX") << std::endl;
}

TEST_F(TSTime, caseTimestampStr)
{
  char buf[128];
  struct timeval epoch =
  { 0, 0 };
  std::cout << "epoch: " << Time::strTimestamp(buf, sizeof(buf), &epoch, false) << std::endl;
  std::cout << "epoch: " << Time::strTimestamp(buf, sizeof(buf), &epoch, true) << std::endl;
  std::cout << "epoch: " << Time::fromTimestamp(buf, sizeof(buf), &(epoch.tv_sec), false) << std::endl;
  std::cout << "epoch: " << Time::fromTimestamp(buf, sizeof(buf), &(epoch.tv_sec), true) << std::endl;
  std::cout << "now:   " << Time::strTimestamp(buf, sizeof(buf), NULL, false) << std::endl;
  std::cout << "now:   " << Time::strTimestamp(buf, sizeof(buf), NULL, true) << std::endl;
  std::cout << "now:   " << Time::fromTimestamp(buf, sizeof(buf), NULL, false) << std::endl;
  std::cout << "now:   " << Time::fromTimestamp(buf, sizeof(buf), NULL, true) << std::endl;
}

TEST_F(TSTime, caseTimeMs)
{
  struct timeval epoch =
  { 0, 0 };
  std::cout << "epoch: " << Time::msTimestamp(&epoch) << std::endl;
  std::cout << "now:   " << Time::msTimestamp() << " ms" << std::endl;
  std::cout << "now:   " << Time::usTimestamp() << " us" << std::endl;
}

TEST_F(TSTime, caseStopWatch)
{
  StopWatch sw;
  sw.start();
  Time::msSleep(200);
  sw.stop();
  std::cout << "time cost: " << sw.timeCostUs() << " us" << std::endl;
}
