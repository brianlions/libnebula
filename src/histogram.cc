/*
 * histogram.cc
 *
 *  Created on: Nov 24, 2011
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
#include "nebula/histogram.h"

namespace nebula
{
  std::string Histogram::toString(uint32_t total_marks) const
  {
    std::string result;
    char buffer[200];

    if (!count_) {
      snprintf(buffer, sizeof(buffer), "----- Histogram: no data available -----\n");
      result.append(buffer);
      return result;
    }

    snprintf(buffer, sizeof(buffer), //
      "----- count: %lu, min: %lu, max: %lu, avg: %lu -----\n", //
      count_, min_, max_, average());
    result.append(buffer);

    double mult = 100.0 / count_;
    uint64_t sum = 0;
    for (int b = 0; b < total_threshold_values_ - 1; b++) {
      if (!buckets_[b]) {
        continue;
      }

      sum += buckets_[b];
      if (b == total_threshold_values_ - 2) {
        snprintf(buffer, sizeof(buffer), "[%11lu,  2 ^ 64 - 1) %10lu %7.3f%% %7.3f%% ", //
          threshold_values_[b], //
          buckets_[b], mult * buckets_[b], mult * sum);
      }
      else {
        snprintf(buffer, sizeof(buffer), "[%11lu, %11lu) %10lu %7.3f%% %7.3f%% ", //
          threshold_values_[b], threshold_values_[b + 1], //
          buckets_[b], mult * buckets_[b], mult * sum);
      }

      result.append(buffer);
      int nm = static_cast<int> (1.0 * (total_marks ? total_marks : num_of_hash_signs) * buckets_[b] / count_ + 0.5);
      result.append(nm, '#');
      result.push_back('\n');
    }

    return result;
  }

  void Histogram::merge(const Histogram& other)
  {
    if (min_ > other.min_) {
      min_ = other.min_;
    }
    if (max_ < other.max_) {
      max_ = other.max_;
    }
    total_ += other.total_;
    count_ += other.count_;
    for (int i = 0; i < total_threshold_values_ - 1; ++i) {
      buckets_[i] += other.buckets_[i];
    }
  }

  void Histogram::clear()
  {
    min_ = 0;
    max_ = 0;
    total_ = 0;
    count_ = 0;
    memset(buckets_, 0, sizeof(buckets_));
  }

  int Histogram::add(uint64_t v)
  {
    if (v > max_) {
      max_ = v;
    }
    if (v < min_) {
      min_ = v;
    }
    total_ += v;
    ++count_;

    // index of last element in `buckets_[]' is `total_threshold_values_ - 2'
    int left = 0, right = total_threshold_values_ - 2;
    int mid = 0;
    while (left <= right) {
      mid = (left + right) / 2;
      if (threshold_values_[mid] <= v && v < threshold_values_[mid + 1]) {
        break;
      }
      else if (v < threshold_values_[mid]) {
        right = mid - 1;
      }
      else {
        left = mid + 1;
      }
    }
    buckets_[mid] += 1;
    return mid;
  }

  const uint64_t Histogram::threshold_values_[] =
  { //
      0, //
      1, 2, 3, 4, 5, //

      6, 7, 8, 9, 10, //
      12, 14, 16, 18, 20, //
      25, 30, 35, 40, 45, 50, //

      60, 70, 80, 90, 100, //
      120, 140, 160, 180, 200, //
      250, 300, 350, 400, 450, 500, //

      600, 700, 800, 900, 1000, //
      1200, 1400, 1600, 1800, 2000, //
      2500, 3000, 3500, 4000, 4500, 5000, //

      6000, 7000, 8000, 9000, 10000, //
      12000, 14000, 16000, 18000, 20000, //
      25000, 30000, 35000, 40000, 45000, 50000, //

      60000, 70000, 80000, 90000, 100000, //
      120000, 140000, 160000, 180000, 200000, //
      250000, 300000, 350000, 400000, 450000, 500000, //

      600000, 700000, 800000, 900000, 1000000, //
      1200000, 1400000, 1600000, 1800000, 2000000, //
      2500000, 3000000, 3500000, 4000000, 4500000, 5000000, //

      6000000, 7000000, 8000000, 9000000, 10000000, //
      12000000, 14000000, 16000000, 18000000, 20000000, //
      25000000, 30000000, 35000000, 40000000, 45000000, 50000000, //

      60000000, 70000000, 80000000, 90000000, 100000000, //
      120000000, 140000000, 160000000, 180000000, 200000000, //
      250000000, 300000000, 350000000, 400000000, 450000000, 500000000, //

      600000000, 700000000, 800000000, 900000000, 1000000000, //
      1200000000, 1400000000, 1600000000, 1800000000, 2000000000, //
      2500000000, 3000000000, 3500000000, 4000000000, 4500000000, 5000000000, //

      6000000000, 7000000000, 8000000000, 9000000000, 10000000000, //
      12000000000, 14000000000, 16000000000, 18000000000, 20000000000, //
      25000000000, 30000000000, 35000000000, 40000000000, 45000000000, 50000000000, //

      ~((uint64_t) 0) //
      };
}
