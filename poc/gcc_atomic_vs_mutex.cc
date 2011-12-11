/*
 * gcc_atomic_vs_mutex.cc
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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "nebula/time.h"

void evaluate(unsigned int * v, int num_iterations = 1000000)
{
  nebula::StopWatch sw;
  int i = 0;
  sw.start();
  for (i = 0; i < num_iterations; ++i) {
    if (i & 0x1) {
      __sync_val_compare_and_swap(v, 1, 0);
    }
    else {
      __sync_val_compare_and_swap(v, 0, 1);
    }
  }
  sw.stop();
  printf("%s\n", sw.message("__sync_val_compare_and_swap()", num_iterations));
}

void evaluateVolatile(volatile unsigned int * v, int num_iterations = 1000000)
{
  nebula::StopWatch sw;
  int i = 0;
  sw.start();
  for (i = 0; i < num_iterations; ++i) {
    if (i & 0x1) {
      __sync_val_compare_and_swap(v, 1, 0);
    }
    else {
      __sync_val_compare_and_swap(v, 0, 1);
    }
  }
  sw.stop();
  printf("%s\n", sw.message("volatile __sync_val_compare_and_swap()", num_iterations));
}

void evaluatePthreadMutex(pthread_mutex_t * lock, int num_iterations = 1000000)
{
  nebula::StopWatch sw;
  int i = 0;
  sw.start();
  for (i = 0; i < num_iterations; ++i) {
    if (i & 0x1) {
      pthread_mutex_unlock(lock);
    }
    else {
      pthread_mutex_lock(lock);
    }
  }
  sw.stop();
  if (!(i & 0x1)) {
    pthread_mutex_unlock(lock);
  }
  printf("%s\n", sw.message("pthread_mutex_{lock,unlock}", num_iterations));
}

int main(int argc, char ** argv)
{
  int num_iterations = 1000000;
  if (argc >= 2) {
    num_iterations = atoi(argv[1]);
  }

  unsigned int ui = 0;
  volatile unsigned int vui = 0;
  evaluate(&ui, num_iterations);
  evaluateVolatile(&vui, num_iterations);
  pthread_mutex_t lock;
  pthread_mutex_init(&lock, NULL);
  evaluatePthreadMutex(&lock, num_iterations);
  pthread_mutex_destroy(&lock);

  exit(0);
}
