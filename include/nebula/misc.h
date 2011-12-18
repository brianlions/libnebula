/*
 * misc.h
 *
 *  Created on: Dec 18, 2011
 *  Author:     Brian Y. ZHANG
 *  Email:      brianlions at gmail dot com
 */

#ifndef _BrianZ_NEBULA_MISC_H_
#define _BrianZ_NEBULA_MISC_H_

#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "nebula/attributes.h"

namespace nebula
{
  class Misc
  {
  public:
    /*
     * NOTES:
     *   NOT thread safe, IPv4 only
     */
    static int nameToIp(const char * name_or_ip, char * buf, size_t buflen) DEPRECATED
    {
      struct hostent * ent = gethostbyname(name_or_ip);
      if (!ent) {
        return -1;
      }
      else {
        if (inet_ntop(AF_INET, ent->h_addr_list[0], buf, buflen)) {
          return 0;
        }
        return -1;
      }
    }

    /*
     * Notes: `fd' must be in non-block mode.
     */
    static size_t readAllBytes(int fd)
    {
      char buf[1024];
      size_t total = 0;
      ssize_t nr;
      while ((nr = read(fd, buf, sizeof(buf))) > 0) {
        total += nr;
      }
      return total;
    }
  };
}

#endif /* _BrianZ_NEBULA_MISC_H_ */
