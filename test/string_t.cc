/*
 * string_t.cc
 *
 *  Created on: Dec 13, 2011
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
#include <gtest/gtest.h>
#include "nebula/string.h"

using nebula::String;

class StringTS: public testing::Test
{
protected:
  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

TEST_F(StringTS, caseStartsWithOrEndsWith)
{
  EXPECT_TRUE(String::startsWith("hello world", "hello"));
  EXPECT_TRUE(String::startsWith("hello world", ""));
  EXPECT_FALSE(String::startsWith("hello world", "good night"));
  EXPECT_FALSE(String::startsWith("hello", "hello world"));
  EXPECT_TRUE(String::endsWith("hello world", "world"));
  EXPECT_TRUE(String::endsWith("hello world", ""));
  EXPECT_FALSE(String::endsWith("hello world", "night"));
  EXPECT_FALSE(String::endsWith("wo", "world"));
}

TEST_F(StringTS, caseUpperLower)
{
  char s[] = "abcdEFGxyz123";
  EXPECT_STREQ("abcdefgxyz123", String::toLowerCase(s));
  EXPECT_STREQ("ABCDEFGXYZ123", String::toUpperCase(s));
}
