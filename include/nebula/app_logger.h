/*
 * app_log.h
 *
 *  Created on: Dec 17, 2011
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

#ifndef _BrianZ_NEBULA_APP_LOG_H_
#define _BrianZ_NEBULA_APP_LOG_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "nebula/string.h"
#include "nebula/thread.h"
#include "nebula/pretty_message.h"
#include "nebula/time.h"

namespace nebula
{
  namespace LogModule
  {
    struct LogLevel
    {
      // named after openlog(), syslog(), closelog()
      enum Constants
      {
        EMERG = 0, ALERT = 1, CRIT = 2, ERR = 3, WARNING = 4, NOTICE = 5, INFO = 6, DEBUG = 7
      };

      const char * toString(int c, bool abbr = false)
      {
        switch (c) {
          case EMERG:
            return abbr ? "eme" : "emerg";
          case ALERT:
            return abbr ? "ale" : "alert";
          case CRIT:
            return abbr ? "cri" : "crit";
          case ERR:
            return "err"; // no alias
          case WARNING:
            return abbr ? "war" : "warning";
          case NOTICE:
            return abbr ? "not" : "notice";
          case INFO:
            return abbr ? "inf" : "info";
          case DEBUG:
            return abbr ? "deb" : "debug";
          default:
            return abbr ? "<?>" : "<unknown>";
        }
      }

      static int logMask(int level)
      {
        return (level >= EMERG && level <= DEBUG) ? (1 << level) : 0;
      }

      static int logUpto(int level)
      {
        return (level >= EMERG && level <= DEBUG) ? ((1 << (level + 1)) - 1) : 0;
      }
    }; /* struct LogLevel */

    namespace internal
    {
      class FileHandle
      {
      private:
        char * prefix_;
        char log_file_[PATH_MAX];
        char format_[NAME_MAX + 1];
        int fd_;
        unsigned num_write_;
        unsigned max_write_;
        size_t path_len_;

      public:
        FileHandle(const char * log_dir = NULL, const char * name_prefix = NULL, unsigned frequency = 1) :
          prefix_(NULL), fd_(-1), num_write_(0), max_write_(frequency), path_len_(0)
        {
          log_file_[0] = '\0';
          format_[0] = '\0';
          if (log_dir) {
            path_len_ = String::strlcpy(log_file_, log_dir, sizeof(log_file_));
            if (!String::endsWith(log_file_, "/")) {
              path_len_ = String::strlcat(log_file_, "/", sizeof(log_file_));
            }

            DEV_MESSAGE("dirname: `%s'", log_file_);
          }
          if (path_len_) {
            String::strlcpy(format_, name_prefix, sizeof(format_));
            String::strlcat(format_, "_%Y.%m.%d.%H.%M.%S_XXXXXX", sizeof(format_));
            DEV_MESSAGE("basename template: `%s'", format_);
            char basename[NAME_MAX + 1];
            Time::formatDataTime(basename, sizeof(basename), format_);
            String::strlcat(log_file_, basename, sizeof(log_file_));
            DEV_MESSAGE("realpath template: `%s'", log_file_);
          }
          else {
            DEV_MESSAGE("realpath: `/dev/tty'");
          }
          if (path_len_) {
            if ((fd_ = mkstemp(log_file_)) >= 0) {
              DEV_MESSAGE("realpath: `%s'", log_file_);
            }
          }
          else {
            fd_ = open("/dev/tty", O_WRONLY);
          }
          // TODO log to sys log if fd_ < 0?
        }

        ~FileHandle()
        {
          if (fd_ > 0) {
            if (path_len_ && num_write_) {
              fsync(fd_);
            }

            DEV_MESSAGE("closing file `%s'", (path_len_ ? log_file_ : "/dev/tty"));
            close(fd_);
          }
        }

        bool switchFile()
        {
          if (path_len_) {
            fsyncToDisk();
            log_file_[path_len_] = '\0';
            DEV_MESSAGE("dirname: `%s'", log_file_);
            close(fd_);
            fd_ = -1;

            char basename[NAME_MAX + 1];
            Time::formatDataTime(basename, sizeof(basename), format_);
            String::strlcat(log_file_, basename, sizeof(log_file_));
            DEV_MESSAGE("realpath template: `%s'", log_file_);
            if ((fd_ = mkstemp(log_file_)) < 0) {
              // TODO log to syslog?
            }
            else {
              DEV_MESSAGE("realpath: `%s'", log_file_);
            }
          }
          return false;
        }

        void fsyncToDisk()
        {
          if (fd_ > 0 && path_len_ && num_write_ == max_write_) {
            DEV_MESSAGE("fsync(fd_:%d) ...", fd_);
            fsync(fd_);
            num_write_ = 0;
          }
        }

        ssize_t writeMessage(const char * message, size_t length)
        {
          ssize_t nw = -1;
          if (fd_ >= 0 && (nw = write(fd_, message, length)) >= 0) {
            //            DEV_MESSAGE("%ld bytes written to file descriptor %d", nw, fd_);
            ++num_write_;
            fsyncToDisk();
          }
          return nw;
        }
      }; /* class FileHandle */

    } /* namespace internal */

  } /* namespace LogModule */
} /* namespace nebula */

#endif /* _BrianZ_NEBULA_APP_LOG_H_ */
