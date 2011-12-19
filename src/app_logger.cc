/*
 * app_logger.cc
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

#include "nebula/app_logger.h"

namespace nebula
{
  namespace LogModule
  {
    volatile int LogServer::ready_to_serve_client_ = 0;
    pthread_mutex_t LogServer::uniq_instance_lock_ = PTHREAD_MUTEX_INITIALIZER;
    LogServer * LogServer::uniq_instance_ = NULL;

    namespace internal
    {
      FileHandle::~FileHandle()
      {
        if (fd_ > 0) {
          if (path_len_ && num_write_) {
            fsync(fd_);
          }

          DEV_MESSAGE("closing file `%s'", (path_len_ ? log_file_ : "/dev/tty"));
          close(fd_);
        }
      }

      void FileHandle::setConfig(const char * log_dir, const char * name_prefix, unsigned frequency)
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

      bool FileHandle::switchFile()
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
    } /* namespace internal */

    LogHandle::~LogHandle()
    {
      if (conn_fd_ >= 0) {
        close(conn_fd_);
      }
      if (buffer_) {
        free(buffer_);
      }
    }

    void LogHandle::setLogLevel(LogLevel::Constants level)
    {
      if (level >= LogLevel::EMERG && level <= LogLevel::DEBUG) {
        log_level_ = level;
      }
    }

    ssize_t LogHandle::log(LogLevel::Constants level, const char * format, ...)
    {
      if (!buffer_) {
        if (!(buffer_ = (char *) malloc(MAX_MESSAGE_LENDTH))) {
          return -1;
        }
        buffer_size_ = MAX_MESSAGE_LENDTH;
      }
      if (level > log_level_) {
        return 0;
      }
      char timestamp[64];
      int len;
      int delta;
      Time::strTimestamp(timestamp, sizeof(timestamp));
      if ((len = snprintf(buffer_, buffer_size_, //
        "[%s %s %s]\t", timestamp, LogLevel::toString(level, true), identity_)) < 0) {
        return -1;
      }
      va_list ap;
      va_start(ap, format);
      delta = vsnprintf(buffer_ + len, buffer_size_ - len, format, ap);
      va_end(ap);
      if (delta >= static_cast<int> (buffer_size_ - len)) {
        len = sizeof(buffer_) - 1; // buffer is terminated by '\0'
      }
      else {
        len += delta;
      }
      if (len == sizeof(buffer_) - 1) {
        buffer_[len] = '\n';
        len++;
      }
      else if (buffer_[len - 1] != '\n') {
        buffer_[len] = '\n';
        len++;
      }
      return send(conn_fd_, buffer_, static_cast<size_t> (len), 0);
    }

    int LogServer::writeLogMessage(LogLevel::Constants level, const char * format, ...)
    {
      char timestamp[64];
      char buffer[4096];
      int len;
      int delta;
      Time::strTimestamp(timestamp, sizeof(timestamp));
      if ((len = snprintf(buffer, sizeof(buffer), //
        "[%s %s LogServer] ", timestamp, LogLevel::toString(level, true))) < 0) {
        return -1;
      }
      va_list ap;
      va_start(ap, format);
      delta = vsnprintf(buffer + len, sizeof(buffer) - len, format, ap);
      va_end(ap);
      if (delta >= static_cast<int> (sizeof(buffer) - len)) {
        len = sizeof(buffer) - 1; // buffer is terminated by '\0'
      }
      else {
        len += delta;
      }
      if (len == sizeof(buffer) - 1) {
        buffer[len] = '\n';
        len++;
      }
      else if (buffer[len - 1] != '\n') {
        buffer[len] = '\n';
        len++;
      }
      return static_cast<int> (file_handle_.writeMessage(buffer, len));
    }

    uint32_t LogServer::checkStopFlag(uint64_t time_event_id, void * data, AsyncEvent * ae)
    {
      LogServer * self = reinterpret_cast<LogServer*> (data);
      if (self->getStopFlag()) {
        self->writeLogMessage(LogLevel::ERR, "%s:%d:%s(): LogServer is stopping", //
          __FILE__, __LINE__, __FUNCTION__);
        clearReadyToServe();
        self->async_event_.stop();
        return 0;
      }
      else {
        return DEFAULT_CHECK_INTERVAL_MS;
      }
    }

    void LogServer::destroyInstance()
    {
      LogServer::getInstance()->setStopFlag();
      LogServer::getInstance()->join();
      pthread_mutex_lock(&uniq_instance_lock_);
      delete uniq_instance_;
      uniq_instance_ = NULL;
      pthread_mutex_unlock(&uniq_instance_lock_);
    }

    void * LogServer::routine()
    {
      char errbuf[64];
      if ((domain_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): socket(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return (void *) errno;
      }
      if (AsyncIo::setNonblock(domain_fd_) < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): AsyncIo::setNonblock(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return (void *) errno;
      }
      server_addr_.sun_family = AF_UNIX;
      snprintf(server_addr_.sun_path, sizeof(server_addr_.sun_path), "/tmp/p%d_t%lx", getpid(), self());
      if (bind(domain_fd_, (const struct sockaddr *) &server_addr_,
        sizeof(server_addr_.sun_family) + strlen(server_addr_.sun_path)) < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): bind(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return (void *) errno;
      }
      if (async_event_.initialize() < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): AsyncEvent::initialize(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return (void *) errno;
      }
      if (async_event_.addFdEvent(domain_fd_, AsyncEvent::READABLE, recvDomainSocketMessage, this) < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): AsyncEvent::addFdEvent(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return (void *) errno;
      }
      if (async_event_.addTimeEvent(DEFAULT_CHECK_INTERVAL_MS, checkStopFlag, this) < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): AsyncEvent::addTimeEvent(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return (void *) errno;
      }
      setReadyToServe();
      writeLogMessage(LogLevel::NOTICE, "%s:%d:%s(): LogServer is ready", //
        __FILE__, __LINE__, __FUNCTION__);
      async_event_.eventLoop();

      return (void *) 0;
    }

    LogHandle * LogServer::createConnection(const char * identity, LogLevel::Constants log_level)
    {
      char errbuf[64];
      int conn_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
      if (conn_fd < 0) {
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): socket(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return NULL;
      }
      if (connect(conn_fd, (const struct sockaddr *) &server_addr_,
        sizeof(server_addr_.sun_family) + strlen(server_addr_.sun_path)) < 0) {
        close(conn_fd);
        writeLogMessage(LogLevel::ERR, "%s:%d:%s(): connect(): `%s'", //
          __FILE__, __LINE__, __FUNCTION__, String::strerror(errno, errbuf, sizeof(errbuf)));
        return NULL;
      }
      LogHandle * result = new LogHandle(identity, log_level);
      result->setConnFd(conn_fd);
      return result;
    }

  } /* namespace LogModule */
} /* namespace nebula */
