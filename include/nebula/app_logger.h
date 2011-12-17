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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdarg.h>
#include "nebula/attributes.h"
#include "nebula/async_event.h"
#include "nebula/async_io.h"
#include "nebula/string.h"
#include "nebula/thread.h"
#include "nebula/pretty_message.h"
#include "nebula/time.h"
#include "nebula/standard.h"

#define APPLOG_QUIT(handle, fmt, ...)    do {        \
  (handle)->log(nebula::LogLevel::ERR,               \
  "%s:%d:%s(): " fmt " exit!",                       \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
  nebula::Time::msSleep(1000); /* this is ugly! */   \
  exit(EXIT_FAILURE);                                \
} while (0)

#define APPLOG_DEBUG(handle, fmt, ...)   do { (handle)->log(nebula::LogModule::LogLevel::DEBUG,   "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_INFO(handle, fmt, ...)    do { (handle)->log(nebula::LogModule::LogLevel::INFO,    "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_NOTICE(handle, fmt, ...)  do { (handle)->log(nebula::LogModule::LogLevel::NOTICE,  "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_WARNING(handle, fmt, ...) do { (handle)->log(nebula::LogModule::LogLevel::WARNING, "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_ERR(handle, fmt, ...)     do { (handle)->log(nebula::LogModule::LogLevel::ERR,     "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_CRIT(handle, fmt, ...)    do { (handle)->log(nebula::LogModule::LogLevel::CRIT,    "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_ALERT(handle, fmt, ...)   do { (handle)->log(nebula::LogModule::LogLevel::ALERT,   "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)
#define APPLOG_EMERG(handle, fmt, ...)   do { (handle)->log(nebula::LogModule::LogLevel::EMERG,   "%s:%d:%s(): " fmt, \
  __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); } while (0)

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

      static const char * toString(int c, bool abbr = false)
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
        FileHandle() :
          prefix_(NULL), fd_(-1), num_write_(0), max_write_(0), path_len_(0)
        {

        }

        void setConfig(const char * log_dir = NULL, const char * name_prefix = NULL, unsigned frequency = 1);

        ~FileHandle();

        bool switchFile();

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

    class LogHandle: public Standard::NoCopy
    {
      friend class LogServer;
    private:
      enum
      {
        MAX_MESSAGE_LENDTH = (64 << 10)
      };
      LogLevel::Constants log_level_;
      int conn_fd_;
      char identity_[64];
      char * buffer_;
      size_t buffer_size_;

      void setConnFd(int fd)
      {
        conn_fd_ = fd;
      }

    public:
      LogHandle(const char * identity, LogLevel::Constants level) :
        log_level_(level), conn_fd_(-1), buffer_(NULL), buffer_size_(0)
      {
        String::strlcpy(identity_, identity, sizeof(identity_));
      }

      ~LogHandle();

      void setLogLevel(LogLevel::Constants level);

      LogLevel::Constants logLevle() const
      {
        return log_level_;
      }

      ssize_t log(LogLevel::Constants level, const char * format, ...) CLASS_PRINTF_FORMAT(2, 3);
    }; /* class LogHandle */

    class LogServer: public nebula::Thread
    {
    private:
      static volatile int ready_to_serve_client_;
      static pthread_mutex_t uniq_instance_lock_;
      static LogServer * uniq_instance_;

      enum
      {
        DEFAULT_CHECK_INTERVAL_MS = 1000, //
          MAX_DGRAM_LENGTH = (64 << 10)
      };

      char domain_sock_path_[PATH_MAX];
      int domain_fd_;
      struct sockaddr_un server_addr_;
      internal::FileHandle file_handle_;
      AsyncEvent async_event_;

    private:
      LogServer(const char * log_dir = NULL, const char * log_file_prefix = NULL, unsigned frequency = 1) :
        domain_fd_(-1)
      {
        domain_sock_path_[0] = '\0';
        memset(&server_addr_, 0, sizeof(server_addr_));
        file_handle_.setConfig(log_dir, log_file_prefix, frequency);
      }

      ~LogServer()
      {
        if (domain_fd_ >= 0) {
          close(domain_fd_);
        }
        if (strlen(server_addr_.sun_path)) {
          unlink(server_addr_.sun_path);
        }
      }

      int writeLogMessage(LogLevel::Constants level, const char * format, ...) CLASS_PRINTF_FORMAT(2, 3);

      static void setReadyToServe()
      {
        __sync_val_compare_and_swap(&ready_to_serve_client_, 0, 1);
      }

      static void clearReadyToServe()
      {
        __sync_val_compare_and_swap(&ready_to_serve_client_, 1, 0);
      }

      static uint32_t recvDomainSocketMessage(int fd, uint32_t mask, void * data, AsyncEvent * ae)
      {
        LogServer * self = reinterpret_cast<LogServer*> (data);
        char message_buf[MAX_DGRAM_LENGTH];
        ssize_t len;
        while ((len = recvfrom(self->domain_fd_, message_buf, sizeof(message_buf), 0, NULL, NULL)) > 0) {
          self->file_handle_.writeMessage(message_buf, len);
        }
        return AsyncEvent::NONE;
      }

      static uint32_t checkStopFlag(uint64_t time_event_id, void * data, AsyncEvent * ae);

    public:
      /*
       * Description:
       *   Start the log server thread.
       * Return value;
       *   Non NULL pointer if the log server thread is successfully started, or NULL if error.
       */
      static LogServer * getInstance(const char * log_dir = NULL, const char * log_file_prefix = NULL,
        unsigned frequency = 1)
      {
        if (!uniq_instance_) {
          pthread_mutex_lock(&uniq_instance_lock_);
          if (!uniq_instance_) {
            LogServer * temp = new LogServer(log_dir, log_file_prefix, frequency);
            int rc;
            if ((rc = temp->create())) {
              delete temp;
              temp = NULL;
            }
            else {
              while (!isReadyToServe()) {
                Time::msSleep(10);
              }
            }
            uniq_instance_ = temp;
          }
          pthread_mutex_unlock(&uniq_instance_lock_);
        }
        return uniq_instance_;
      } /* getInstance() */

      static void destroyInstance();

      virtual void * routine();

      static bool isReadyToServe()
      {
        return __sync_fetch_and_add(&ready_to_serve_client_, 0) == 1;
      }

      LogHandle * createConnection(const char * identity, LogLevel::Constants log_level);

    }; /* class LogServer */

    INLINE void startLogServer(const char * log_dir, const char * log_file_prefix, unsigned frequency = 1)
    {
      LogServer::getInstance(log_dir, log_file_prefix, frequency);
    }

    INLINE void stopLogServer()
    {
      LogServer::destroyInstance();
    }

    INLINE LogHandle * getLogHandle(const char * identity, LogLevel::Constants log_level = LogLevel::INFO)
    {
      LogServer * ls UNUSED = LogServer::getInstance();
      return ls->createConnection(identity, log_level);
    }

  } /* namespace LogModule */

} /* namespace nebula */

#endif /* _BrianZ_NEBULA_APP_LOG_H_ */
