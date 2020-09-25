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

#include <memory>
#include <string>
#include <vector>

#include "glog/logging.h"

#include "common/constants.h"
#include "common/io/input/input_block_info_cache.h"

namespace axe {
namespace common {

struct BlockInfo {
  std::string file_name;
  std::string host_name;
  size_t offset;
};

class AbstractInputBlockInfo {
 public:
  static std::shared_ptr<AbstractInputBlockInfo> Create(const std::string& url, FS protocol);

  explicit AbstractInputBlockInfo(const std::string& url) { urls_.push_back(url); }
  explicit AbstractInputBlockInfo(const std::vector<std::string>& urls) { urls_ = urls; }
  virtual ~AbstractInputBlockInfo() {}

  virtual void FetchBlocksInfo() = 0;
  const std::vector<BlockInfo>& GetBlocks() const { return blocks_; }
  virtual double GetBlockSize(const std::string& file, size_t offset) const = 0;

  inline const auto& GetUrl() const {
    DCHECK_GT(urls_.size(), 0);
    return urls_[0];
  }
  inline const auto& GetUrls() const { return urls_; }

  void SetInputBlockInfoCache(InputBlockInfoCache* cache) { cache_ = cache; }

 protected:
  // file_name, host, offset
  std::vector<BlockInfo> blocks_;
  std::vector<std::string> urls_;

  InputBlockInfoCache* cache_;  // owned by job manager
};

using InputBlockInfo = AbstractInputBlockInfo;

}  // namespace common
}  // namespace axe
