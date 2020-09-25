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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "glog/logging.h"

namespace axe {
namespace common {

class Timer {
 public:
  Timer(int time_interval_ms) : time_interval_(time_interval_ms) {}

  void Start() {
    thread_.reset(new std::thread([=]() {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait_for(lock, std::chrono::milliseconds(time_interval_), [&] { return !is_running_; });  // Delay to avoid first dead result.
      while (is_running_) {
        cv_.wait_for(lock, std::chrono::milliseconds(time_interval_), [&] { return !is_running_; });
        if (is_running_) {
          handler_();
        }
      }
    }));
  }

  ~Timer() {
    std::unique_lock<std::mutex> lock(mtx_);
    is_running_ = false;
    lock.unlock();
    cv_.notify_all();
    if (thread_ != nullptr) {
      thread_->join();
    }
  }

  void SetHandler(const std::function<void()>& handler) { handler_ = handler; }

 private:
  int time_interval_;
  std::function<void()> handler_;
  std::atomic_bool is_running_{true};
  std::mutex mtx_;
  std::condition_variable cv_;
  std::unique_ptr<std::thread> thread_;
};

}  // namespace common
}  // namespace axe
