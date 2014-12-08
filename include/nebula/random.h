/*
 * random.h
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

#ifndef _BrianZ_NEBULA_RANDOM_H_
#define _BrianZ_NEBULA_RANDOM_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace nebula
{
  typedef class Prng
  {
  private:
    char statebuf_[64];
    struct random_data databuf_;

  public:
    Prng(unsigned int seed = 1)
    {
      initialize(seed);
    }

    void initialize(unsigned int seed = 1)
    {
      memset(&statebuf_, 0, sizeof(statebuf_));
      memset(&databuf_, 0, sizeof(databuf_));
      initstate_r(seed, statebuf_, sizeof(statebuf_), &databuf_);
      setstate_r(statebuf_, &databuf_);
      srandom_r(seed, &databuf_);
    }

    int32_t randomI32()
    {
      return (int32_t) randomU32();
    }

    uint32_t randomU32()
    {
      uint32_t r, r2;
      // XXX random_r generate 31 bits integer
      random_r(&databuf_, (int32_t *) &r);
      random_r(&databuf_, (int32_t *) &r2);
      return ((r << 19) | (r2 >> 12));
    }

    uint64_t randomU64()
    {
      uint32_t a = randomU32();
      uint32_t b = randomU32();
      uint64_t r = a;
      r <<= 32;
      r |= b;
      return r;
    }

    int64_t randomI64()
    {
      return (int64_t) randomU64();
    }
  } PseudoRandomNumberGenerator;
}

#endif /* _BrianZ_NEBULA_RANDOM_H_ */
