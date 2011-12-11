/*
 * hardwareinfo.h
 *
 *  Created on: Dec 8, 2011
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

#ifndef _BrianZ_NEBULA_HARDWAREINFO_H_
#define _BrianZ_NEBULA_HARDWAREINFO_H_

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

namespace nebula
{
  class HardwareInfo
  {
  private:
    static pthread_mutex_t lock_;
    static int done_;

    static char cpu_model_name_[1024];
    static uint64_t num_of_processors_;
    static uint64_t cpu_cache_size_;
    static uint64_t memory_total_;
    static uint64_t swap_total_;

    static void rightTrim(char * line);
    static void parseCpuinfo(char ** line_buf, size_t * line_size);
    static void parseMemInfo(char ** line_buf, size_t * line_size);
    static void parseProcFiles();
    static void checkAndParse();

  public:
    static const char * cpuModelName()
    {
      checkAndParse();
      return cpu_model_name_;
    }

    static uint64_t numOfProcessors()
    {
      checkAndParse();
      return num_of_processors_;
    }

    static uint64_t cpuCacheSize()
    {
      checkAndParse();
      return cpu_cache_size_;
    }

    static uint64_t memoryTotal()
    {
      checkAndParse();
      return memory_total_;
    }

    static uint64_t swapTotal()
    {
      checkAndParse();
      return swap_total_;
    }
  };
}

#endif /* _BrianZ_NEBULA_HARDWAREINFO_H_ */
