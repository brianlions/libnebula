/*
 * async_io.h
 *
 *  Created on: Dec 9, 2011
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

#ifndef _BrianZ_NEBULA_ASYNC_IO_H_
#define _BrianZ_NEBULA_ASYNC_IO_H_

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace nebula
{
  class AsyncIo
  {
  private:
    enum
    {
      ERROR = -1, OK = 0
    };

  public:
    enum
    {
      AIO_NONE = 0x00, AIO_CLOEXEC = 0x01, AIO_NONBLOCK = 0x02
    };

    // Generic --------------------------------------------------------------------------------------------------------

    static int setNonblock(int fd)
    {
      int flags;
      if ((flags = fcntl(fd, F_GETFL)) < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return ERROR;
      }
      return OK;
    }

    /*
     * Return value:
     *  -1      error
     *  0       blocking
     *  1       non-blocking
     */
    static int isNonblock_3state(int fd)
    {
      int flags;
      if ((flags = fcntl(fd, F_GETFL)) < 0) {
        return ERROR;
      }
      else if (flags & O_NONBLOCK) {
        return 1;
      }
      else {
        return 0;
      }
    }

    static int setCloseOnExec(int fd)
    {
      int flags;
      if ((flags = fcntl(fd, F_GETFD)) < 0 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
        return ERROR;
      }
      return OK;
    }

    /*
     * Return value:
     *  -1      error
     *  0       close-on-exec is not set
     *  1       close-on-exec is set
     */
    static int isCloseOnExec_3state(int fd)
    {
      int flags;
      if ((flags = fcntl(fd, F_GETFD)) < 0) {
        return ERROR;
      }
      else if (flags & FD_CLOEXEC) {
        return 1;
      }
      else {
        return 0;
      }
    }

    static int pipe2(int pipefd[2], int flags = AIO_CLOEXEC | AIO_NONBLOCK);

    // TCP/IP specific ------------------------------------------------------------------------------------------------

    static int setTcpNoDelay(int sockfd)
    {
      int flag = 1;
      if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        return ERROR;
      }
      return OK;
    }

    static int isTcpNoDelay_3state(int sockfd)
    {
      int flag;
      socklen_t len = sizeof(flag);
      if (getsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, &len) < 0) {
        return ERROR;
      }
      return flag;
    }

    static int setReuseAddr(int sockfd)
    {
      int flag = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        return ERROR;
      }
      return OK;
    }

    static int isReuseAddr_3state(int sockfd)
    {
      int flag;
      socklen_t len = sizeof(flag);
      if (getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, &len) < 0) {
        return ERROR;
      }
      return flag;
    }

    static int tcpServer(const char * bind_ip, int bind_port)
    {
      return createAsyncServerSocket(bind_ip, bind_port, 1);
    }

    static int tcpServer(const struct sockaddr_in * bind_addr)
    {
      return createAsyncServerSocket(bind_addr, 1);
    }

    static int udpServer(const char * bind_ip, int bind_port)
    {
      return createAsyncServerSocket(bind_ip, bind_port, 0);
    }

    static int udpServer(const struct sockaddr_in * bind_addr)
    {
      return createAsyncServerSocket(bind_addr, 0);
    }

    static int accept4(int tcp_server_sockfd, struct sockaddr * client_addr, socklen_t * client_addr_len,
      int flags = AIO_CLOEXEC | AIO_NONBLOCK);

    static int tcpClient(const char * serv_ip, int serv_port)
    {
      return createAsyncClientSocket(serv_ip, serv_port, 1);
    }

    static int tcpClient(const struct sockaddr_in * serv_addr)
    {
      return createAsyncClientSocket(serv_addr, 1);
    }

    static int udpClient(const char * serv_ip, int serv_port)
    {
      return createAsyncClientSocket(serv_ip, serv_port, 0);
    }

    static int udpClient(const struct sockaddr_in * serv_addr)
    {
      return createAsyncClientSocket(serv_addr, 0);
    }

  private:
    AsyncIo();
    ~AsyncIo();
    static int createAsyncServerSocket(const char * bind_ip, int bind_port, int use_tcp);
    static int createAsyncServerSocket(const struct sockaddr_in * bind_addr, int use_tcp);
    static int createAsyncClientSocket(const char * serv_ip, int serv_port, int use_tcp);
    static int createAsyncClientSocket(const struct sockaddr_in * serv_addr, int use_tcp);
  };
}

#endif /* _BrianZ_NEBULA_ASYNC_IO_H_ */
