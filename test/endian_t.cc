/*
 * endian_t.cc
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
#include <arpa/inet.h>
#include <iostream>
#include <gtest/gtest.h>
#include "nebula/endian.h"

using nebula::Endian;

TEST(EndianTS, caseSimple)
{
  if (Endian::isBigEndian()) {
    std::cout << "host is using BIG ENDIAN" << std::endl;
  }
  if (Endian::isLittleEndian()) {
    std::cout << "host is using LITTLE ENDIAN" << std::endl;
  }

  uint16_t ua = 0x1122U;
  uint32_t ub = 0x11223344U;
  uint64_t uc = 0x1122334455667788UL;

  EXPECT_EQ(htons(ua), Endian::toBe16(ua));
  EXPECT_EQ(htonl(ub), Endian::toBe32(ub));

  if (Endian::isLittleEndian()) {
    EXPECT_EQ(Endian::toBe16(ua), 0x2211U);
    EXPECT_EQ(Endian::toBe32(ub), 0x44332211U);
    EXPECT_EQ(Endian::toBe64(uc), 0x8877665544332211UL);

    EXPECT_EQ(Endian::toLe16(ua), ua);
    EXPECT_EQ(Endian::toLe32(ub), ub);
    EXPECT_EQ(Endian::toLe64(uc), uc);
  }

  if (Endian::isBigEndian()) {
    EXPECT_EQ(Endian::toBe16(ua), ua);
    EXPECT_EQ(Endian::toBe32(ub), ub);
    EXPECT_EQ(Endian::toBe64(uc), uc);

    EXPECT_EQ(Endian::toLe16(ua), 0x2211U);
    EXPECT_EQ(Endian::toLe32(ub), 0x44332211U);
    EXPECT_EQ(Endian::toLe64(uc), 0x8877665544332211UL);
  }

  EXPECT_EQ(Endian::fromBe16(Endian::toBe16(ua)), ua);
  EXPECT_EQ(Endian::fromLe16(Endian::toLe16(ua)), ua);
  EXPECT_EQ(Endian::toBe16(Endian::fromBe16(ua)), ua);
  EXPECT_EQ(Endian::toLe16(Endian::fromLe16(ua)), ua);

  EXPECT_EQ(Endian::fromBe32(Endian::toBe32(ub)), ub);
  EXPECT_EQ(Endian::fromLe32(Endian::toLe32(ub)), ub);
  EXPECT_EQ(Endian::toBe32(Endian::fromBe32(ub)), ub);
  EXPECT_EQ(Endian::toLe32(Endian::fromLe32(ub)), ub);

  EXPECT_EQ(Endian::fromBe64(Endian::toBe64(uc)), uc);
  EXPECT_EQ(Endian::fromLe64(Endian::toLe64(uc)), uc);
  EXPECT_EQ(Endian::toBe64(Endian::fromBe64(uc)), uc);
  EXPECT_EQ(Endian::toLe64(Endian::fromLe64(uc)), uc);
}
