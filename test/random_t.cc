/*
 * random_t.cc
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

#include <stdio.h>
#include <gtest/gtest.h>
#include "nebula/random.h"

using nebula::Prng;

class RandomTS: public testing::Test
{
protected:

  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

TEST_F(RandomTS, caseSimple)
{
  Prng prng(123);
  int count = 5;
  for (int i = 0; i < count; ++i) {
    printf("rand u32: 0x%08x\n", prng.randomU32());
  }
  for (int i = 0; i < count; ++i) {
    printf("rand u64: 0x%016lx\n", prng.randomU64());
  }
}
