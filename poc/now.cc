/*
 * now.cc
 *
 *  Created on: Nov 30, 2011
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "nebula/time.h"

int main(int argc, char ** argv)
{
  int num_iterations = 1000000;
  if (argc >= 2) {
    num_iterations = atoi(argv[1]);
  }

  nebula::StopWatch sw1, sw2;

  struct timeval tv;
  sw1.start();
  for (int i = 0; i < num_iterations; ++i) {
    gettimeofday(&tv, NULL);
  }
  sw1.stop();
  printf("gettimeofday(): %ld usec / %d = %0.6f usec\n", sw1.timeCostUs(), num_iterations,
    1.0 * sw1.timeCostUs() / num_iterations);

  struct timespec ts;
  sw2.start();
  for (int i = 0; i < num_iterations; ++i) {
    clock_gettime(CLOCK_REALTIME, &ts);
  }
  sw2.stop();
  printf("%s\n", sw2.message("clock_gettime()", num_iterations));

  exit(0);
}
