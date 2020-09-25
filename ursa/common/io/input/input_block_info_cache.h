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
#include <unordered_map>
#include <vector>

#include "glog/logging.h"

namespace axe {
namespace common {

class InputBlockInfoCache {
 public:
  struct ExtendedBlockInfo {
    std::string file_name;
    size_t offset;
    std::vector<std::string> host_names;
    size_t block_size;
  };

  std::vector<ExtendedBlockInfo*> LookUpUrl(const std::string& url) {
    std::vector<ExtendedBlockInfo*> ret;
    auto pos = url_to_files_.find(url);
    if (pos != url_to_files_.end()) {
      ret.reserve(pos->second.second - pos->second.first);
      for (auto idx = pos->second.first; idx < pos->second.second; ++idx) {
        ret.push_back(&(browsed_files_[idx]));
      }
    }
    return ret;
  }

  // the url here must be a folder without subfolder
  void AddFilesForUrl(const std::string& url, std::vector<ExtendedBlockInfo>&& files) {
    auto start = browsed_files_.size();
    if (browsed_files_.empty()) {
      browsed_files_ = std::move(files);
    } else {
      browsed_files_.insert(browsed_files_.end(), files.begin(), files.end());
    }
    url_to_files_.insert({url, std::make_pair(start, browsed_files_.size())});
  }

 private:
  std::unordered_map<std::string, std::pair<size_t, size_t>> url_to_files_;
  std::vector<ExtendedBlockInfo> browsed_files_;
};

}  // namespace common
}  // namespace axe
