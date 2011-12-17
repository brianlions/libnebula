/*
 * endian.h
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

#ifndef _BrianZ_NEBULA_ENDIAN_H_
#define _BrianZ_NEBULA_ENDIAN_H_

#include <stdint.h>

namespace nebula
{
  class Endian
  {
  private:
    static uint16_t swap16(uint16_t in)
    {
      return (((in & 0xff) << 8) | (in >> 8));
    }

    static uint32_t swap32(uint32_t in)
    {
      uint32_t out = 0;
      for (int i = 0; i < 4; ++i) {
        out = (out << 8) | (in & 0xff);
        in >>= 8;
      }
      return out;
    }

    static uint64_t swap64(uint64_t in)
    {
      uint64_t out = 0;
      for (int i = 0; i < 8; ++i) {
        out = (out << 8) | (in & 0xff);
        in >>= 8;
      }
      return out;
    }

  public:
    static bool isLittleEndian()
    {
      uint16_t test = 0x1234;
      uint8_t * ptr = (uint8_t *) &test;
      return ptr[0] > ptr[1];
    }

    static bool isBigEndian()
    {
      uint16_t test = 0x1234;
      uint8_t * ptr = (uint8_t *) &test;
      return ptr[0] < ptr[1];
    }

    static uint16_t toBe16(uint16_t h16)
    {
      return isBigEndian() ? h16 : swap16(h16);
    }

    static uint16_t toLe16(uint16_t h16)
    {
      return isLittleEndian() ? h16 : swap16(h16);
    }

    static uint16_t fromBe16(uint16_t be16)
    {
      return isBigEndian() ? be16 : swap16(be16);
    }

    static uint16_t fromLe16(uint16_t le16)
    {
      return isLittleEndian() ? le16 : swap16(le16);
    }

    static uint32_t toBe32(uint32_t h32)
    {
      return isBigEndian() ? h32 : swap32(h32);
    }

    static uint32_t toLe32(uint32_t h32)
    {
      return isLittleEndian() ? h32 : swap32(h32);
    }

    static uint32_t fromBe32(uint32_t be32)
    {
      return isBigEndian() ? be32 : swap32(be32);
    }

    static uint32_t fromLe32(uint32_t le32)
    {
      return isLittleEndian() ? le32 : swap32(le32);
    }

    static uint64_t toBe64(uint64_t h64)
    {
      return isBigEndian() ? h64 : swap64(h64);
    }

    static uint64_t toLe64(uint64_t h64)
    {
      return isLittleEndian() ? h64 : swap64(h64);
    }

    static uint64_t fromBe64(uint64_t be64)
    {
      return isBigEndian() ? be64 : swap64(be64);
    }

    static uint64_t fromLe64(uint64_t le64)
    {
      return isLittleEndian() ? le64 : swap64(le64);
    }
  }; /* class Endian */
}

#endif /* _BrianZ_NEBULA_ENDIAN_H_ */
