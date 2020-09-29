// Copyright 2018 H-AXE
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <memory>
#include <string>

namespace axe {
namespace base {

class Tokenizer {
 public:
  static const char WHITESPACE_DELIMITER[];
  static const char CSV_DELIMITER[];
};

class StrtokTokenizer : public Tokenizer {
  struct ArrayDeleter {
    inline void operator()(char* p) const { delete[] p; }
  };

 public:
  /**
   * Thread-safe tokenizer using strtok_r. Fast but incurs copy.
   *
   * @param str the string to tokenize.
   * @param delimiter the delimiter to split the string. Default is WHITESPACE_DELIMITER.
   */
  explicit StrtokTokenizer(const std::string& str, const std::string delimiter = WHITESPACE_DELIMITER);

  /**
   * Get the next token.
   *
   * @returns the pointer to the next token or nullptr.
   */
  char* next();

 private:
  std::unique_ptr<char, ArrayDeleter> data_;
  const std::string delimiter_;

  char* pos_;      // pos for strtok_r
  char* current_;  // current token
};

/**
 * Whitespace tokenizer using std::isspace. Zero-copy.
 *
 * @param str the string to tokenize.
 */
class WhiteSpaceTokenizer : public Tokenizer {
 public:
  explicit WhiteSpaceTokenizer(const std::string& str) : str_(str) {}
  bool next(std::string& buffer);

 private:
  const std::string& str_;
  size_t idx_ = 0;
  size_t last_ = 0;
};

}  // namespace base
}  //  namespace axe
