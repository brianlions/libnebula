/*
 * test_file_parser_t.cc
 *
 *  Created on: Dec 15, 2011
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
#include "nebula/text_file_parser.h"
#include "nebula/string.h"

using nebula::String;
using nebula::Tokens;

class TextFileParserTS: public testing::Test
{
protected:
  Tokens tokens_;
  char line[128];
  const char * line_template;

  virtual void SetUp()
  {
    line_template = "abc\tdef ghi;jkl\nABCD\n";
    EXPECT_TRUE(String::strlcpy(line, line_template, sizeof(line)) < sizeof(line));
  }

  virtual void TearDown()
  {

  }
};

TEST_F(TextFileParserTS, caseSplitLine01)
{
  tokens_.splitString(line, "\t; ");
  EXPECT_TRUE(tokens_.numOfTokens() == 4);
  EXPECT_STREQ(tokens_.token(0), "abc");
  EXPECT_STREQ(tokens_.token(1), "def");
  EXPECT_STREQ(tokens_.token(2), "ghi");
  EXPECT_STREQ(tokens_.token(3), "jkl\nABCD\n");
}

TEST_F(TextFileParserTS, caseSplitLine02)
{
  tokens_.splitString(line, ";");
  EXPECT_TRUE(tokens_.numOfTokens() == 2);
  EXPECT_STREQ(tokens_.token(0), "abc\tdef ghi");
  EXPECT_STREQ(tokens_.token(1), "jkl\nABCD\n");
}

TEST_F(TextFileParserTS, caseSplitLine03)
{
  tokens_.splitString(line, " \t\n");
  EXPECT_TRUE(tokens_.numOfTokens() == 4);
  EXPECT_STREQ(tokens_.token(0), "abc");
  EXPECT_STREQ(tokens_.token(1), "def");
  EXPECT_STREQ(tokens_.token(2), "ghi;jkl");
  EXPECT_STREQ(tokens_.token(3), "ABCD");
}

TEST_F(TextFileParserTS, caseSplitLineEmptyDelim)
{
  tokens_.splitString(line, "");
  EXPECT_TRUE(tokens_.numOfTokens() == 1);
  EXPECT_STREQ(tokens_.token(0), line_template);
}

TEST_F(TextFileParserTS, caseSplitLineEmptyLine)
{
  EXPECT_TRUE(String::strlcpy(line, "", sizeof(line)) < sizeof(line));
  tokens_.splitString(line, " \t\n");
  EXPECT_TRUE(tokens_.numOfTokens() == 0);
}

//---------------------------------------------------------------------------------------------------------------------
