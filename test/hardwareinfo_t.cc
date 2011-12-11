/*
 * hardwareinfo_t.cc
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

#include <iostream>
#include <gtest/gtest.h>
#include "nebula/hardwareinfo.h"

using nebula::HardwareInfo;

class HardwareInfoTS: public testing::Test
{
protected:
  virtual void SetUp()
  {

  }

  virtual void TearDown()
  {

  }
};

TEST_F(HardwareInfoTS, caseSimple)
{
  std::cout //
      << "CPU:            " << HardwareInfo::numOfProcessors() << " * " << HardwareInfo::cpuModelName() << std::endl //
      << "CPU cache size: " << HardwareInfo::cpuCacheSize() << std::endl //
      << "Memory total:   " << HardwareInfo::memoryTotal() << std::endl //
      << "Swap total:     " << HardwareInfo::swapTotal() << std::endl;
}
