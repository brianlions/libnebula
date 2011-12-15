/*
 * attributes.h
 *
 *  Created on: Nov 23, 2011
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

#ifndef _BrianZ_NEBULA_ATTRIBUTES_H_
#define _BrianZ_NEBULA_ATTRIBUTES_H_

#ifdef __GNUC__
#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#ifdef __GNUC__
#define PRINTF_FORMAT(M, N)       __attribute__((format(printf, (M), (N))))
#define SCANF_FORMAT(M, N)        __attribute__((format(scanf, (M), (N))))
#define STRFTIME_FORMAT(M)        __attribute__((format(strftime, (M), 0)))
#define CLASS_PRINTF_FORMAT(M, N) __attribute__((format(printf, (M + 1), (N + 1))))
#define CLASS_SCANF_FORMAT(M, N)  __attribute__((format(scanf, (M + 1), (N + 1))))
#define CLASS_STRFTIME_FORMAT(M)  __attribute__((format(strftime, (M + 1), 0)))
#define INLINE                    __inline__
#define ALWAYS_INLINE             __attribute__((always_inline))
#define WARN_UNUSED_RESULT        __attribute__((warn_unused_result))
#define UNUSED                    __attribute__((unused))
#define DEPRECATED                __attribute__((deprecated))
#else
#define PRINTF_FORMAT(M, N)
#define SCANF_FORMAT(M, N)
#define STRFTIME_FORMAT(M)
#define CLASS_PRINTF_FORMAT(M, N)
#define CLASS_SCANF_FORMAT(M, N)
#define CLASS_STRFTIME_FORMAT(M)
#define INLINE
#define ALWAYS_INLINE
#define WARN_UNUSED_RESULT
#define UNUSED
#define DEPRECATED
#endif

#endif /* _BrianZ_NEBULA_ATTRIBUTES_H_ */
