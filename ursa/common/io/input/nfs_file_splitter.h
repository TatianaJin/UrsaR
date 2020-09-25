// Copyright 2020 HDL
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

#include <fstream>
#include <memory>
#include <string>

#include "common/io/input/file_splitter.h"

namespace axe {
namespace common {

class NFSFileSplitter : public FileSplitter {
 public:
  explicit NFSFileSplitter(size_t block_size = 128 * 1024 * 1024) : block_size_(block_size) {}
  ~NFSFileSplitter() { file_.close(); }

  void Load(const std::string& url) override;
  std::string FetchBlock(const std::string& fn, size_t offset, bool is_next = false) override;
  std::string_view FetchBlockView(const std::string& fn, size_t offset, bool is_next = false) override;

 protected:
  int ReadBlock(const std::string& fn, size_t offset);

  size_t block_size_;
  std::shared_ptr<char> data_;
  std::ifstream file_;
};

}  // namespace common
}  // namespace axe
