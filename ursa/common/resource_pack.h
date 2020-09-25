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

namespace axe {
namespace common {

class ResourcePack {
 public:
  ResourcePack() = default;
  ResourcePack(double memory, double cpu, double network, double disk) : memory_(memory), cpu_(cpu), network_(network), disk_(disk) {}

  friend base::BinStream& operator<<(base::BinStream& bin_stream, const ResourcePack& res) {
    bin_stream << res.memory_ << res.cpu_ << res.network_ << res.disk_;
    return bin_stream;
  }
  friend base::BinStream& operator>>(base::BinStream& bin_stream, ResourcePack& res) {
    bin_stream >> res.memory_ >> res.cpu_ >> res.network_ >> res.disk_;
    return bin_stream;
  }

  const ResourcePack& operator+=(const ResourcePack& update) {
    memory_ += update.memory_;
    cpu_ += update.cpu_;
    network_ += update.network_;
    disk_ += update.disk_;
    return *this;
  }

  const ResourcePack& operator-=(const ResourcePack& update) {
    memory_ -= update.memory_;
    cpu_ -= update.cpu_;
    network_ -= update.network_;
    disk_ -= update.disk_;
    return *this;
  }

  ResourcePack operator*(int multiplier) const {
    ResourcePack pack = *this;
    pack.memory_ *= multiplier;
    pack.cpu_ *= multiplier;
    pack.network_ *= multiplier;
    pack.disk_ *= multiplier;
    return pack;
  }

  bool ContainedBy(const ResourcePack& other) const {
    return memory_ <= other.memory_ && cpu_ <= other.cpu_ && network_ <= other.network_ && disk_ <= other.disk_;
  }

  ResourcePack Min(const ResourcePack& other) const {
    return ResourcePack(std::max(0., std::min(memory_, other.memory_ - 1e-6)), std::max(0., std::min(cpu_, other.cpu_ - 1e-6)),
                        std::max(0., std::min(network_, other.network_ - 1e-6)), std::max(0., std::min(disk_, other.disk_ - 1e-6)));
  }

  ResourcePack operator/(const ResourcePack& divider) const {
    ResourcePack ret;
    ret.memory_ = memory_ / divider.memory_;
    ret.cpu_ = cpu_ / divider.cpu_;
    ret.network_ = network_ / divider.network_;
    ret.disk_ = disk_ / divider.disk_;
    return ret;
  }

  double DotProduct(const ResourcePack& other) const {
    return memory_ * other.memory_ + cpu_ * other.cpu_ + network_ * other.network_ + disk_ * other.disk_;
  }

  double& Memory() { return memory_; }
  double& CPU() { return cpu_; }
  double& Network() { return network_; }
  double& Disk() { return disk_; }

  double GetMemory() const { return memory_; }
  double GetCPU() const { return cpu_; }
  double GetNetwork() const { return network_; }
  double GetDisk() const { return disk_; }

  std::string ToString() const {
    std::string ret = " Memory: ";
    ret += std::to_string(memory_);
    ret += " CPU: ";
    ret += std::to_string(cpu_);
    ret += " Network: ";
    ret += std::to_string(network_);
    ret += " Disk: ";
    ret += std::to_string(disk_);
    return ret;
  }

  double GetNorm() const { return memory_ + cpu_ + network_ + disk_; }

  ResourcePack GetEstimatedTime(double max_time, double cpu_process_rate, double bandwidth) const {
    ResourcePack ret = *this;
    ret.CPU() = std::min(ret.CPU() / 1024. / 1024. / cpu_process_rate, max_time);
    ret.Network() = std::min(ret.Network() / 1024. / 1024. / bandwidth, max_time);
    return ret;
  }

 protected:
  double memory_ = 0;
  double cpu_ = 0;
  double network_ = 0;
  double disk_ = 0;
};

}  // namespace common
}  // namespace axe
