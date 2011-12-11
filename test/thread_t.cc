/*
 * thread_t.cc
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

#include <gtest/gtest.h>
#include "nebula/thread.h"

using nebula::Thread;

class ThreadTS: public testing::Test
{
protected:

  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

TEST_F(ThreadTS, caseSimple)
{
  class Simple: public Thread
  {
  private:
    void * code_;
  public:
    Simple(void * code = NULL) :
      code_(code)
    {

    }

    void setCode(void * code)
    {
      code_ = code;
    }

    void * routine()
    {
      return code_;
    }
  };

  Simple array[10];
  for (size_t i = 0; i < sizeof(array) / sizeof(array[0]); ++i) {
    array[i].setCode((void *) i);
    array[i].create();
  }
  void * ret = NULL;
  for (size_t i = 0; i < sizeof(array) / sizeof(array[0]); ++i) {
    array[i].join(&ret);
    EXPECT_TRUE(ret == (void *) i);
  }
}
