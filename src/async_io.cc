/*
 * async_io.cc
 *
 *  Created on: Dec 10, 2011
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

#include <string.h>
#include "nebula/async_io.h"
#include "nebula/string.h"

namespace nebula
{
  int AsyncIo::pipe2(int pipefd[2], int flags)
  {
    if (flags & (~(AIO_CLOEXEC | AIO_NONBLOCK))) {
      errno = EINVAL;
      return ERROR;
    }
    if (pipe(pipefd) < 0) {
      return ERROR;
    }
    if ((flags & AIO_NONBLOCK) && (setNonblock(pipefd[0]) == ERROR || setNonblock(pipefd[1]) == ERROR)) {
      close(pipefd[0]);
      close(pipefd[1]);
      return ERROR;
    }
    if ((flags & AIO_CLOEXEC) && (setCloseOnExec(pipefd[0]) == ERROR || setCloseOnExec(pipefd[1]) == ERROR)) {
      close(pipefd[0]);
      close(pipefd[1]);
      return ERROR;
    }
    return OK;
  }

  int AsyncIo::accept4(int tcp_server_sockfd, struct sockaddr * client_addr, socklen_t * client_addr_len, int flags)
  {
    if (flags & (~(AIO_CLOEXEC | AIO_NONBLOCK))) {
      errno = EINVAL;
      return ERROR;
    }
    // in case we need to restore the length
    socklen_t orig_len = 0;
    if (client_addr_len) {
      orig_len = *client_addr_len;
    }
    int fd;
    if ((fd = accept(tcp_server_sockfd, client_addr, client_addr_len)) < 0) {
      return ERROR;
    }
    if (((flags & AIO_NONBLOCK) && setNonblock(fd) < 0) || ((flags & AIO_CLOEXEC) && setCloseOnExec(fd) < 0)) {
      close(fd);
      if (client_addr_len) {
        *client_addr_len = orig_len;
      }
      return ERROR;
    }
    //      if (setTcpNoDelay(fd) < 0) {
    //        close(fd);
    //        if (client_addr_len) {
    //          *client_addr_len = orig_len;
    //        }
    //        return ERROR;
    //      }
    return fd;
  }

  int AsyncIo::createAsyncServerSocket(const char * bind_ip, int bind_port, int use_tcp)
  {
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(bind_port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind_ip && inet_pton(AF_INET, bind_ip, &(bind_addr.sin_addr)) <= 0) {
      return ERROR;
    }
    return createAsyncServerSocket(&bind_addr, use_tcp);
  }

  int AsyncIo::createAsyncServerSocket(const struct sockaddr_in * bind_addr, int use_tcp)
  {
    int fd;
    if ((fd = socket(AF_INET, use_tcp ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0) {
      return ERROR;
    }
    if (setNonblock(fd) == ERROR || setReuseAddr(fd) == ERROR) {
      close(fd);
      return ERROR;
    }
    if (bind(fd, (const struct sockaddr *) bind_addr, sizeof(*bind_addr)) < 0) {
      close(fd);
      return ERROR;
    }
    if (use_tcp && listen(fd,
#ifdef SOMAXCONN
      SOMAXCONN
#else
      64
#endif
        ) < 0) {
      close(fd);
      return ERROR;
    }
    return fd;
  }

  int AsyncIo::createAsyncClientSocket(const char * serv_ip, int serv_port, int use_tcp)
  {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    if (inet_pton(AF_INET, serv_ip, &(serv_addr.sin_addr)) <= 0) {
      return ERROR;
    }
    return createAsyncClientSocket(&serv_addr, use_tcp);
  }

  int AsyncIo::createAsyncClientSocket(const struct sockaddr_in * serv_addr, int use_tcp)
  {
    int fd;
    if ((fd = socket(AF_INET, use_tcp ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0) {
      return ERROR;
    }
    if (use_tcp && connect(fd, (const struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
      close(fd);
      return ERROR;
    }
    if (setNonblock(fd) == ERROR) {
      close(fd);
      return ERROR;
    }
    return fd;
  }

  int AsyncIo::createAsyncUnixServerSocket(const char * fs_pathname, int use_tcp)
  {
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    if (String::strlcpy(serv_addr.sun_path, fs_pathname, sizeof(serv_addr.sun_path)) >= sizeof(serv_addr.sun_path)) {
      return ERROR;
    }
    int fd;
    if ((fd = socket(AF_UNIX, use_tcp ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0) {
      return ERROR;
    }
    if (setNonblock(fd) == ERROR) {
      close(fd);
      return ERROR;
    }
    if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      close(fd);
      return ERROR;
    }
    return fd;
  }

  int AsyncIo::createAsyncUnixClientSocket(const char * fs_pathname, int use_tcp)
  {
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    if (String::strlcpy(serv_addr.sun_path, fs_pathname, sizeof(serv_addr.sun_path)) >= sizeof(serv_addr.sun_path)) {
      return ERROR;
    }
    int fd;
    if ((fd = socket(AF_UNIX, use_tcp ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0) {
      return ERROR;
    }
    if (setNonblock(fd) == ERROR) {
      close(fd);
      return ERROR;
    }
    if (connect(fd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      close(fd);
      return ERROR;
    }
    return fd;
  }
}
