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

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "glog/logging.h"

#include "base/bin_stream.h"
#include "common/closure.h"
#include "common/constants.h"

namespace axe {
namespace common {

class EventActor {
 public:
  virtual ~EventActor() {}
  void RegisterHandler(uint32_t type, std::function<void(std::shared_ptr<base::BinStream>)> func) { handlers_[type] = func; }

  void HandleEvent(uint32_t type, const std::shared_ptr<base::BinStream>& bin_stream) {
    if (handlers_.count(type) > 0) {
      handlers_.at(type)(bin_stream);
    } else {
      LOG(ERROR) << "Unknown event type " << type;
    }
  }

  virtual void RegisterBasicHandler() = 0;

 private:
  std::map<uint32_t, std::function<void(const std::shared_ptr<base::BinStream>&)>> handlers_;
};

}  // namespace common
}  // namespace axe
