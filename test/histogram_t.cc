/*
 * histogram_t.cc
 *
 *  Created on: Nov 24, 2011
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
#include "nebula/histogram.h"

using nebula::Histogram;

class TSHistogram: public testing::Test
{
protected:
  Histogram hist_;

  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {
    hist_.clear();
  }
};

TEST_F(TSHistogram, caseAddValue)
{
  EXPECT_EQ(hist_.add(0), 0);
  EXPECT_EQ(hist_.add(1), 1);
  EXPECT_EQ(hist_.add(2), 2);
  EXPECT_EQ(hist_.add(10), 10);
  EXPECT_EQ(hist_.add(11), 10);
  EXPECT_EQ(hist_.add(12), 11);
  EXPECT_EQ(hist_.add(20), 15);
  EXPECT_EQ(hist_.add(21), 15);
  EXPECT_EQ(hist_.add(24), 15);
  EXPECT_EQ(hist_.add(25), 16);
  EXPECT_EQ(hist_.add(50), 21);
  EXPECT_EQ(hist_.add(59), 21);

  EXPECT_EQ(hist_.add(45000000000L), 164);
  EXPECT_EQ(hist_.add(49999999999L), 164);
  EXPECT_EQ(hist_.add(50000000000L), 165);
  EXPECT_EQ(hist_.add(0xffffFFFFffffFFFFL), 165);

  std::cout << hist_.toString() << std::endl;
}

TEST_F(TSHistogram, caseMinMaxAvg)
{
  std::cout << hist_.toString(50) << std::endl;

  hist_.add(5);
  hist_.add(10);
  hist_.add(11);
  hist_.add(15);
  EXPECT_EQ(hist_.min(), (uint64_t) 5);
  EXPECT_EQ(hist_.max(), (uint64_t) 15);
  EXPECT_EQ(hist_.average(), (uint64_t) 41 / 4);
  EXPECT_EQ(hist_.count(), (uint64_t) 4);
  EXPECT_EQ(hist_.total(), (uint64_t) 41);

  std::cout << hist_.toString(50) << std::endl;
}
