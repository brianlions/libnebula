/*
 * hardwareinfo.cc
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

#include "nebula/hardwareinfo.h"

namespace nebula
{
  pthread_mutex_t HardwareInfo::lock_ = PTHREAD_MUTEX_INITIALIZER;
  int HardwareInfo::done_ = 0;

  char HardwareInfo::cpu_model_name_[1024];
  uint64_t HardwareInfo::num_of_processors_ = 0;
  uint64_t HardwareInfo::cpu_cache_size_ = 0;
  uint64_t HardwareInfo::memory_total_ = 0;
  uint64_t HardwareInfo::swap_total_ = 0;

  void HardwareInfo::rightTrim(char * line)
  {
    size_t len = strlen(line);
    size_t i = 0;
    int quit = 0;
    for (i = len - 1; i >= 0 && !quit; i--) {
      switch (*(line + i)) {
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
          *(line + i) = '\0';
          break;
        default:
          quit = 1;
          break;
      }
    }
  }

  void HardwareInfo::parseCpuinfo(char ** line_buf, size_t * line_size)
  {
#ifdef __linux__
    FILE * fin = fopen("/proc/cpuinfo", "r");
    if (!fin) { // FIXME
      return;
    }
    char * pos = NULL;
    while (getline(line_buf, line_size, fin) != -1) {
      if (strstr(*line_buf, "model name") == *line_buf) {
        ++num_of_processors_;
        rightTrim(*line_buf);
        if ((pos = strstr(*line_buf, ": "))) {
          String::strlcpy(cpu_model_name_, pos + 2, sizeof(cpu_model_name_));
        }
      }
      else if (strstr(*line_buf, "cache size") == *line_buf) {
        rightTrim(*line_buf);
        if ((pos = strstr(*line_buf, ": "))) {
          cpu_cache_size_ = String::strToU64(pos + 2);
          if (strchr(pos, 'k') || strchr(pos, 'K')) {
            cpu_cache_size_ *= (1 << 10);
          }
          else if (strchr(pos, 'm') || strchr(pos, 'M')) {
            cpu_cache_size_ *= (1 << 20);
          }
        }
      }
    }
    fclose(fin);
#endif // __linux__
  }

  void HardwareInfo::parseMemInfo(char ** line_buf, size_t * line_size)
  {
#ifdef __linux__
    FILE * fin = fopen("/proc/meminfo", "r");
    if (!fin) { // FIXME
      return;
    }
    while (getline(line_buf, line_size, fin) != -1) {
      if (strstr(*line_buf, "MemTotal:") == *line_buf) {
        memory_total_ = String::strToU64(*line_buf + strlen("MemTotal:"));
        if (strchr(*line_buf, 'k') || strchr(*line_buf, 'k')) {
          memory_total_ *= (1 << 10);
        }
        else if (strchr(*line_buf, 'm') || strchr(*line_buf, 'M')) {
          memory_total_ *= (1 << 20);
        }
      }
      if (strstr(*line_buf, "SwapTotal:") == *line_buf) {
        swap_total_ = String::strToU64(*line_buf + strlen("SwapTotal:"));
        if (strchr(*line_buf, 'k') || strchr(*line_buf, 'k')) {
          swap_total_ *= (1 << 10);
        }
        else if (strchr(*line_buf, 'm') || strchr(*line_buf, 'M')) {
          swap_total_ *= (1 << 20);
        }
      }
    }
    fclose(fin);
#endif // __linux__
  }

  void HardwareInfo::parseProcFiles()
  {
#ifdef __linux__
    size_t line_size = 1024;
    char * line_buf = (char *) malloc(line_size);
    if (!line_buf) { // FIXME
      return;
    }
    parseCpuinfo(&line_buf, &line_size);
    parseMemInfo(&line_buf, &line_size);
    free(line_buf);
#endif // __linux__
  }

  void HardwareInfo::checkAndParse()
  {
    if (!done_) {
      pthread_mutex_lock(&lock_);
      if (!done_) {
        String::strlcpy(cpu_model_name_, "unknown", sizeof(cpu_model_name_));
        parseProcFiles();
        done_ = 1;
      }
      pthread_mutex_unlock(&lock_);
    }
  }
}
