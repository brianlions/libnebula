/*
 * string_funcs.cc
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

#include "nebula/string.h"

namespace nebula
{
  int64_t String::strToI64(const char * s, bool parse_suffix)
  {
    char * ptr = NULL;
    long int val = strtol(s, &ptr, 10);
    if (val == LONG_MIN || val == LONG_MAX) {
      return val;
    }
    if (ptr && parse_suffix) {
      switch (*ptr) {
        case 'K':
          val *= (1 << 10);
          break;
        case 'k':
          val *= 1000;
          break;
        case 'M':
          val *= (1 << 20);
          break;
        case 'm':
          val *= 1000000;
          break;
        case 'G':
          val *= (1 << 30);
          break;
        case 'g':
          val *= 1000000000;
          break;
        default:
          break;
      }
    }
    return (int64_t) val;
  }

  uint64_t String::strToU64(const char * s, bool parse_suffix)
  {
    char * ptr = NULL;
    unsigned long int val = strtoul(s, &ptr, 10);
    if (val == ULONG_MAX) {
      return val;
    }
    if (ptr && parse_suffix) {
      switch (*ptr) {
        case 'K':
          val *= (1 << 10);
          break;
        case 'k':
          val *= 1000;
          break;
        case 'M':
          val *= (1 << 20);
          break;
        case 'm':
          val *= 1000000;
          break;
        case 'G':
          val *= (1 << 30);
          break;
        case 'g':
          val *= 1000000000;
          break;
        default:
          break;
      }
    }
    return (uint64_t) val;
  }

  bool String::startsWith(const char * src_str, const char * prefix)
  {
    size_t la, lb;
    if ((la = strlen(src_str)) < (lb = strlen(prefix))) {
      return false;
    }
    return strncmp(src_str, prefix, lb) == 0;
  }

  bool String::endsWith(const char * src_str, const char * suffix)
  {
    size_t la, lb;
    if ((la = strlen(src_str)) < (lb = strlen(suffix))) {
      return false;
    }
    return strncmp(src_str + la - lb, suffix, lb) == 0;
  }

  char * String::toUpperCase(char * input)
  {
    size_t len = strlen(input);
    for (size_t i = 0; i < len; ++i) {
      input[i] = toupper(input[i]);
    }
    return input;
  }

  char * String::toLowerCase(char * input)
  {
    size_t len = strlen(input);
    for (size_t i = 0; i < len; ++i) {
      input[i] = tolower(input[i]);
    }
    return input;
  }
}
