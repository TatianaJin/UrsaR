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

#include "hdfs/hdfs.h"

#include "common/io/input/file_splitter.h"

namespace axe {
namespace common {

class HDFSFileSplitter : public FileSplitter {
 public:
  ~HDFSFileSplitter();

  void InitBlocksize(hdfsFS fs, const std::string& url);
  void Load(const std::string& url) override;
  std::string FetchBlock(const std::string& fn, size_t offset, bool is_next = false) override;
  std::string_view FetchBlockView(const std::string& fn, size_t offset, bool is_next = false) override;
  size_t GetBloSize() { return hdfs_block_size_; }

 protected:
  int ReadBlock(const std::string& fn, size_t offset);

  std::shared_ptr<char> data_;
  hdfsFS fs_;
  hdfsFile file_ = NULL;

  size_t hdfs_block_size_ = 0;
};

}  // namespace common
}  // namespace axe
