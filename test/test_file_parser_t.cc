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
using nebula::TextFileParser;

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

class TextFileParserTS2: public testing::Test
{
protected:
  char fname[128];
  const char * name_template;
  const char * contents;

  virtual void SetUp()
  {
    name_template = "testsuite_TextFileParser_XXXXXX";
    contents = "grpA 1 2\n"
      "grpA 3\n"
      "grpB 4\n"
      "grpB 5\n"
      "grpA 6 7\n"
      "DONE\n" /* trailing lines skipped in function compute() */
      "grpB 8 9 10\n";

    EXPECT_TRUE(String::strlcpy(fname, name_template, sizeof(fname)) < sizeof(fname));
    int fd = mkstemp(fname);
    EXPECT_TRUE(fd >= 0);

    EXPECT_EQ((size_t) write(fd, contents, strlen(contents)), strlen(contents));
    EXPECT_TRUE(close(fd) == 0);
    //        printf("temp file `%s' created\n", fname);
  }

  virtual void TearDown()
  {
    EXPECT_TRUE(unlink(fname) == 0);
    //        printf("temp file `%s' removed\n", fname);
  }
};

struct GroupSum
{
  int sum_a;
  int sum_b;
  GroupSum() :
    sum_a(0), sum_b(0)
  {
  }
};

int compute(const char * filename, uint64_t num, const Tokens * tokens, void * data)
{
  struct GroupSum * sum = (struct GroupSum *) data;
  if (!tokens->numOfTokens()) {
    return 0;
  }
  else if (tokens->numOfTokens() == 1) {
    if (!strcasecmp(tokens->token(0), "done")) {
      EXPECT_EQ(num, (uint64_t) 6);
      return 1;
    }
  }
  else {
    if (!strcasecmp(tokens->token(0), "grpA")) {
      for (size_t i = 1; i < tokens->numOfTokens(); ++i) {
        sum->sum_a += atoi(tokens->token(i));
      }
    }
    if (!strcasecmp(tokens->token(0), "grpB")) {
      for (size_t i = 1; i < tokens->numOfTokens(); ++i) {
        sum->sum_b += atoi(tokens->token(i));
      }
    }
  }
  return 0;
}

TEST_F(TextFileParserTS2, caseParseFile)
{
  GroupSum sum;
  TextFileParser tfp;
  EXPECT_EQ(1, tfp.parseLineByLine(fname, compute, &sum, " \n"));
  EXPECT_EQ(sum.sum_a, 19);
  EXPECT_EQ(sum.sum_b, 9);
}
