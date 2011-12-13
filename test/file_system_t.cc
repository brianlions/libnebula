/*
 * file_system_t.cc
 *
 *  Created on: Dec 13, 2011
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
#include <iostream>
#include <gtest/gtest.h>
#include "nebula/file_system.h"

using nebula::FileSystem;

class FileSystemTS: public testing::Test
{
protected:
  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

int printItemName(const char * path, const struct stat * sb, void * arg)
{
  std::cout << "\t" << path << (S_ISDIR(sb->st_mode) ? "/" : (S_ISLNK(sb->st_mode) ? "@" : "")) << std::endl;
  return 0;
}

TEST_F(FileSystemTS, caseListHomeAndRoot)
{
  std::cout << "contents of /home:" << std::endl;
  FileSystem::fileTreeTraversal("/home", printItemName, NULL);
  std::cout << std::endl;
  std::cout << "contents of /:" << std::endl;
  FileSystem::fileTreeTraversal("/", printItemName, NULL);
}
