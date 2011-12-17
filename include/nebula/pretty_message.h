/*
 * pretty_message.h
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

#ifndef _BrianZ_NEBULA_PRETTY_MESSAGE_H_
#define _BrianZ_NEBULA_PRETTY_MESSAGE_H_

#include <stdio.h>
#include "nebula/time.h"

#if defined(USE_PRETTY_MESSAGE)
#define PRETTY_MESSAGE(stream, fmt, ...) \
do { \
    char time_str[64]; \
    (void) fprintf((stream), "[%s %s:%04d:%s] " fmt "\n", nebula::Time::strTimestamp(time_str, sizeof(time_str)), \
      __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
} while (0)
#define DEV_MESSAGE(fmt, ...) \
do { \
    char time_str[64]; \
    (void) fprintf(stderr, "[%s %s:%04d:%s] " fmt "\n", nebula::Time::strTimestamp(time_str, sizeof(time_str)), \
      __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
} while (0)
#if 0
#define BEGIN_FUNC() do { fprintf(stderr, "+++++ %s:%d:%s +++\n", __FILE__, __LINE__, __FUNCTION__); } while (0)
#define END_FUNC()   do { fprintf(stderr, "----- %s:%d:%s ---\n", __FILE__, __LINE__, __FUNCTION__); } while (0)
#else
#define BEGIN_FUNC() PRETTY_MESSAGE(stderr, "entering ...")
#define END_FUNC()   PRETTY_MESSAGE(stderr, "leaving ...")
#endif
#else //---------------------------------------------------------------------------------------------------------------
#define PRETTY_MESSAGE(stream, fmt, ...)
#define DEV_MESSAGE(fmt, ...)
#define BEGIN_FUNC()
#define END_FUNC()
#endif

#endif /* _BrianZ_NEBULA_PRETTY_MESSAGE_H_ */
