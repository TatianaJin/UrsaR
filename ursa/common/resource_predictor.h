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
#include <vector>

namespace axe {
namespace common {

class ResourcePredictor {
 public:
  ResourcePredictor() {
    produce_memory_predictor_ = [](const std::vector<double>& input_size) {
      double ret = 0;
      for (double size : input_size) {
        ret += size;
      }
      return ret;
    };
    resource_usage_factor_ = 1.;
  }
  ResourcePredictor(const std::function<double(const std::vector<double>&)>& produce_memory_predictor)
      : produce_memory_predictor_(produce_memory_predictor) {
    resource_usage_factor_ = 1.;
  }
  template <typename Lambda>
  void SetProduceMemoryPredictor(Lambda lambda) {
    produce_memory_predictor_ = lambda;
  }

  double PredictProduceMemory(const std::vector<double>& input_size) const { return produce_memory_predictor_(input_size); }
  double PredictPeakMemory(const std::vector<double>& input_size) const {
    // TODO
    return produce_memory_predictor_(input_size);
  }
  double PredictResourceUsage(double output_data_size) const { return output_data_size * resource_usage_factor_; }
  void SetResourceUsageFactor(double factor) { resource_usage_factor_ = factor; }

 private:
  std::function<double(const std::vector<double>&)> produce_memory_predictor_;
  double resource_usage_factor_;
};

}  // namespace common
}  // namespace axe
