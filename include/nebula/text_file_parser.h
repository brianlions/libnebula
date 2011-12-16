/*
 * text_file_parser.h
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

#ifndef _BrianZ_NEBULA_TEXT_FILE_PARSER_H_
#define _BrianZ_NEBULA_TEXT_FILE_PARSER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nebula/standard.h"

namespace nebula
{
  class Tokens: public Standard::NoCopy
  {
  private:
    const char ** tokens_;
    char * input_string_;
    uint32_t num_of_tokens_;
    uint32_t max_tokens_;
    uint32_t hint_size_;

  public:
    enum
    {
      DEFAULT_HINT_SIZE = 32
    };

    Tokens(uint32_t hint_size = DEFAULT_HINT_SIZE) :
      tokens_(NULL), input_string_(NULL), num_of_tokens_(0), max_tokens_(0),
          hint_size_(hint_size ? hint_size : DEFAULT_HINT_SIZE)
    {

    }

    ~Tokens()
    {
      if (tokens_) {
        free(tokens_);
      }
    }

    void clear()
    {
      input_string_ = NULL;
      num_of_tokens_ = 0;
    }

    bool splitString(char * input, const char * delim = " \f\n\r\t\v")
    {
      clear();
      char * token = NULL;
      char * saved = NULL;
      char * str = input;
      while ((token = strtok_r(str, delim, &saved))) {
        if (!saveTokenAddr(token)) {
          return false;
        }
        str = NULL;
      }
      return true;
    }

    uint32_t numOfTokens() const
    {
      return num_of_tokens_;
    }

    const char * token(uint32_t idx) const
    {
      return (idx < num_of_tokens_) ? (*(tokens_ + idx)) : NULL;
    }

  private:
    bool saveTokenAddr(char * ptr)
    {
      if (num_of_tokens_ == max_tokens_) {
        if (!max_tokens_) {
          max_tokens_ = hint_size_;
        }
        else {
          max_tokens_ += hint_size_;
        }
        const char ** all;
        if (!tokens_) {
          if (!(all = (const char **) malloc(max_tokens_ * sizeof(tokens_[0])))) {
            return false;
          }
        }
        else {
          if (!(all = (const char **) realloc(tokens_, max_tokens_ * sizeof(tokens_[0])))) {
            max_tokens_ -= hint_size_;
            return false;
          }
        }
        tokens_ = all;
      }
      *(tokens_ + num_of_tokens_++) = ptr;
      return true;
    }
  }; /* class Tokens */

  class TextFileParser: Standard::NoCopy
  {
  private:
    enum
    {
      DEFAULT_LINE_SIZE = 4096
    };

    char * line_buffer_;
    size_t line_size_;
    FILE * file_handle_;

  public:
    TextFileParser() :
      line_buffer_(NULL), line_size_(0), file_handle_(NULL)
    {

    }

    ~TextFileParser()
    {
      clear();
    }

    void clear()
    {
      if (line_buffer_) {
        free(line_buffer_);
        line_buffer_ = NULL;
      }
      if (line_size_) {
        line_size_ = 0;
      }
      if (file_handle_) {
        fclose(file_handle_);
        file_handle_ = NULL;
      }
    }

    int parseLineByLine(const char * filename,
      int(*fn)(const char * filename, uint64_t line_num, const Tokens * tokens, void * arg), void * fn_arg = NULL,
      const char * delim = " \f\n\r\t\v", bool skip_empty = true, uint32_t hint_size = Tokens::DEFAULT_HINT_SIZE)
    {
      if (file_handle_) {
        fclose(file_handle_);
        file_handle_ = NULL;
      }

      if (!line_buffer_ && (line_buffer_ = (char *) malloc(DEFAULT_LINE_SIZE)) == NULL) {
        return -1;
      }
      line_size_ = DEFAULT_LINE_SIZE;
      if (!(file_handle_ = fopen(filename, "r"))) {
        return -1;
      }

      int rc = 0;
      uint64_t line_num = 0;
      ssize_t length;
      Tokens tokens(hint_size);
      while ((length = getline(&line_buffer_, &line_size_, file_handle_)) != -1) {
        ++line_num;
        // remove trailing "\r", "\n" or "\r\n"
        if (line_buffer_[length - 1] == '\r' || line_buffer_[length - 1] == '\n') {
          line_buffer_[--length] = '\0';
          if (length && line_buffer_[length - 1] == '\r') {
            line_buffer_[--length] = '\0';
          }
        }
        tokens.splitString(line_buffer_, delim);
        if (!tokens.numOfTokens() && skip_empty) {
          continue;
        }
        if ((rc = (*fn)(filename, line_num, &tokens, fn_arg))) {
          break;
        }
      }
      fclose(file_handle_);
      file_handle_ = NULL;
      return rc;
    }
  }; /* class TextFileParser */

} /* namespace nebula */

#endif /* _BrianZ_NEBULA_TEXT_FILE_PARSER_H_ */
