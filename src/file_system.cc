/*
 * file_system.cc
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

#include "nebula/file_system.h"

namespace nebula
{
  int FileSystem::fileTreeTraversal(const char * dir_path, //
    int(*fn)(const char * fpath, const struct stat * sb, void * arg), //
    void * fn_last_arg)
  {
    int rc = 0;
    DIR * dir;
    struct dirent entry;
    struct dirent * result;
    struct stat entry_stat;
    char entry_path[PATH_MAX];
    size_t used;
    if (!(dir = opendir(dir_path))) {
      return -1;
    }
    used = String::strlcpy(entry_path, dir_path, sizeof(entry_path));
    if (!String::endsWith(entry_path, "/")) {
      used = String::strlcat(entry_path, "/", sizeof(entry_path));
    }
    while (!readdir_r(dir, &entry, &result) && result) {
      if (!strcmp(entry.d_name, "..") || !strcmp(entry.d_name, ".")) {
        continue;
      }
      entry_path[used] = '\0';
      String::strlcat(entry_path, entry.d_name, sizeof(entry_path));
      if ((rc = lstat(entry_path, &entry_stat))//
          || (rc = (*fn)(entry_path, &entry_stat, fn_last_arg))) {
        break;
      }
    }
    closedir(dir);
    return rc;
  }

}
