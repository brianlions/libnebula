/*
 * histogram.h
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

#ifndef _BrianZ_NEBULA_HISTOGRAM_H_
#define _BrianZ_NEBULA_HISTOGRAM_H_

#include <stdint.h>
#include <string.h>
#include <string>
#include <sstream>

namespace nebula
{
  class Histogram
  {
  private:
    enum
    {
      total_threshold_values_ = (6 + 16 * 10 + 1), num_of_hash_signs = 200
    };
    static const uint64_t threshold_values_[total_threshold_values_];

    uint64_t min_;
    uint64_t max_;
    uint64_t total_;
    uint64_t count_;
    // numbers in `threshold_values_[]' divided [0, 2^64) into `total_threshold_values_ -1' zones
    uint64_t buckets_[total_threshold_values_ - 1];

  public:
    Histogram() :
      min_(~((uint64_t) 0)), max_(0), total_(0), count_(0)
    {
      memset(buckets_, 0, sizeof(buckets_));
    }

    ~Histogram()
    {

    }

    std::string toString(uint32_t total_marks = num_of_hash_signs) const;

    void merge(const Histogram& other);

    void clear();

    int add(uint64_t v);

    uint64_t min() const
    {
      return min_;
    }

    uint64_t max() const
    {
      return max_;
    }

    uint64_t average() const
    {
      return count_ ? (total_ / count_) : 0;
    }

    uint64_t total() const
    {
      return total_;
    }

    uint64_t count() const
    {
      return count_;
    }
  };
}

#endif /* _BrianZ_NEBULA_HISTOGRAM_H_ */
