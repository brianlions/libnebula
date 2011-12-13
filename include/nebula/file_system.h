/*
 * file_system.h
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

#ifndef _BrianZ_NEBULA_FILE_SYSTEM_H_
#define _BrianZ_NEBULA_FILE_SYSTEM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include "nebula/string.h"

namespace nebula
{
  class FileSystem
  {
  private:
    FileSystem();
    ~FileSystem();

  public:
    static int mkdir(const char * pathname, bool recursive = false, const char * mode = "rwxr-xr-x");
    static int rmdir(const char * pathname, bool recursive = false);

    static int getFileStatus(const char * path, struct stat * buf, bool follow_link = false)
    {
      if ((follow_link && stat(path, buf) < 0) || lstat(path, buf) < 0) {
        return -1;
      }
      return 0;
    }

    static time_t atime(const char * path, bool follow_link = false)
    {
      struct stat buf;
      if ((follow_link && stat(path, &buf) < 0) || lstat(path, &buf) < 0) {
        return ((time_t) -1);
      }
      return buf.st_atime;
    }

    static time_t ctime(const char * path, bool follow_link = false)
    {
      struct stat buf;
      if ((follow_link && stat(path, &buf) < 0) || lstat(path, &buf) < 0) {
        return ((time_t) -1);
      }
      return buf.st_ctime;
    }

    static time_t mtime(const char * path, bool follow_link = false)
    {
      struct stat buf;
      if ((follow_link && stat(path, &buf) < 0) || lstat(path, &buf) < 0) {
        return ((time_t) -1);
      }
      return buf.st_mtime;
    }

    static int fileTreeTraversal(const char * dir_path, //
      int(*fn)(const char * fpath, const struct stat * sb, void * arg), //
      void * fn_last_arg);
  };
}
#endif /* _BrianZ_NEBULA_FILE_SYSTEM_H_ */
