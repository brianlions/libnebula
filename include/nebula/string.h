/*
 * string.h
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

#ifndef _BrianZ_NEBULA_STRING__H_
#define _BrianZ_NEBULA_STRING__H_

#include <sys/types.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

namespace nebula
{
  class String
  {
  public:
    static size_t strlcat(char * dst, const char * src, size_t siz);
    static size_t strlcpy(char * dst, const char * src, size_t siz);
    static int64_t strToI64(const char * s, bool parse_suffix = false);
    static uint64_t strToU64(const char * s, bool parse_suffix = false);

    static bool startsWith(const char * src_str, const char * prefix);
    static bool endsWith(const char * src_str, const char * suffix);
    static char * toUpperCase(char * input);
    static char * toLowerCase(char * input);
  };
}

#endif /* _BrianZ_NEBULA_STRING__H_ */
