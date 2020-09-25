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

#include <string>

#include "base/bin_stream.h"
#include "common/constants.h"

namespace axe {
namespace common {

using axe::base::BinStream;

struct JobDesc {
  JobDesc() {}
  JobDesc(const std::string& _jm_file, const std::string& _jp_file, const std::string& _config_file)
      : jm_file(_jm_file), jp_file(_jp_file), config_file(_config_file) {}

  friend BinStream& operator<<(BinStream& bin_stream, const JobDesc& job_desc) {
    return bin_stream << job_desc.jm_file << job_desc.jp_file << job_desc.config_file;
  }

  friend BinStream& operator>>(BinStream& bin_stream, JobDesc& job_desc) {
    return bin_stream >> job_desc.jm_file >> job_desc.jp_file >> job_desc.config_file;
  }

  std::string jm_file, jp_file, config_file;
};

}  // namespace common
}  // namespace axe
