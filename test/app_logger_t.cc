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

#include <gtest/gtest.h>
#include "nebula/app_logger.h"

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
  lm::internal::FileHandle fh(".", "foo", 5);
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
  lm::internal::FileHandle fh(NULL);
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
