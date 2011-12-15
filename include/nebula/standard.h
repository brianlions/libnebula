/*
 * standard.h
 *
 *  Created on: Dec 15, 2011
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

#ifndef _BrianZ_NEBULA_STANDARD_H_
#define _BrianZ_NEBULA_STANDARD_H_

namespace nebula
{
  namespace Standard
  {
    class NoAssign
    {
      void operator=(const NoAssign&);
    public:
#if __GNUC__
      NoAssign()
      {
      }
#endif
    };

    class NoCopy: public NoAssign
    {
      NoCopy(const NoCopy&);
    public:
#if __GNUC__
      NoCopy()
      {
      }
#endif
    };
  }
}

#endif /* _BrianZ_NEBULA_STANDARD_H_ */
