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

#include <string.h>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <vector>

#include "glog/logging.h"

#include "common/engine.h"

using DataObj = std::pair<std::vector<std::pair<int, double>>, double>;
using KVDataset = axe::common::Dataset<std::pair<int, double>>;
using KVDatasetPartition = axe::common::DatasetPartition<std::pair<int, double>>;

DatasetPartition<DataObj> LibsvmParse(const std::string& line) {
  DataObj this_obj;
  char* pos;
  std::unique_ptr<char[]> record_ptr(new char[line.size() + 1]);
  strncpy(record_ptr.get(), line.data(), line.size());
  record_ptr.get()[line.size()] = '\0';
  char* tok = strtok_r(record_ptr.get(), " \t:", &pos);

  int i = -1;
  int idx;
  double val;
  while (tok != NULL) {
    if (i == 0) {
      idx = std::atoi(tok) - 1;
      CHECK(idx >= 0) << "idx ERROR !!!!!!!!!!!!!!" << idx;
      i = 1;
    } else if (i == 1) {
      val = std::atof(tok);
      this_obj.first.push_back(std::make_pair(idx, val));
      i = 0;
    } else {
      this_obj.second = std::atof(tok);
      i = 0;
    }
    tok = strtok_r(NULL, " \t:", &pos);
  }
  std::sort(this_obj.first.begin(), this_obj.first.end());
  DatasetPartition<DataObj> ret;
  ret.push_back(this_obj);
  return ret;
}

class LogicalRegression : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    auto input = config->GetOrSet("data", "");
    int n_partitions = std::stoi(config->GetOrSet("parallelism", "20"));
    int n_iters = std::stoi(config->GetOrSet("n_iters", "5"));
    double batch_factor = std::stof(config->GetOrSet("batch_factor", "1"));
    size_t num_dims = std::stoi(config->GetOrSet("num_dims", "0"));
    double alpha = std::stof(config->GetOrSet("alpha", "1.0"));
    int n_partitions2 = std::stoi(config->GetOrSet("partition", "140"));

    LOG(INFO) << batch_factor << "  " << num_dims << "  " << alpha;
    size_t block_dims = num_dims / n_partitions2 + 1;

    auto get_gradient = [batch_factor, alpha](const DatasetPartition<DataObj>& data, const KVDatasetPartition& model) {
      auto time0 = std::chrono::steady_clock::now();
      int start = 0;
      if (batch_factor < 1) {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(0, data.size() - 1);
        start = dist(mt);
      }

      KVDatasetPartition ret;
      if (data.size() == 0)
        return ret;

      size_t batch_size = (size_t)(batch_factor * data.size());
      std::map<int, double> gradients;
      for (int i = 0; i < batch_size; ++i) {
        auto& x = data.at(start).first;
        auto y = std::max(data.at(start).second, 0.);
        double pred_y = 0.0;
        for (auto& field : x) {
          pred_y += model.at(field.first).second * field.second;
        }
        pred_y = 1. / (1. + exp(-1 * pred_y));
        for (auto& field : x) {
          gradients[field.first] += alpha * field.second * (y - pred_y);
        }
        start++;
        start %= data.size();
      }

      ret.reserve(gradients.size());
      for (auto& pair : gradients) {
        ret.push_back(pair);
      }

      auto time1 = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
      LOG(INFO) << "gradient computation " << elapsed << " on " << batch_size << " records with model size " << model.size();
      return ret;
    };

    auto initial_gradient = [batch_factor, alpha, num_dims](const DatasetPartition<DataObj>& data) {
      auto time0 = std::chrono::steady_clock::now();
      int start = 0;
      if (batch_factor < 1) {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(0, data.size() - 1);
        start = dist(mt);
      }

      KVDatasetPartition ret;
      for (int i = 0; i < num_dims; ++i) {
        ret.push_back(std::make_pair(i, 0.));
      }
      if (data.size() == 0)
        return ret;

      size_t batch_size = (size_t)(batch_factor * data.size());
      for (int i = 0; i < batch_size; ++i) {
        auto& x = data.at(start).first;
        auto y = std::max(data.at(start).second, 0.);
        int j = 0;
        for (const auto& field : x) {
          while (j < field.first) {
            ++j;
          }
          ret.at(j).second += alpha * field.second * (y - 0.5);
        }
        start++;
        start %= data.size();
      }
      auto time1 = std::chrono::steady_clock::now();
      LOG(INFO) << "initial gradient " << std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
      return ret;
    };

    auto test_error = [](const DatasetPartition<DataObj>& data, const KVDatasetPartition& model) {
      KVDatasetPartition ret;
      int count = 0;
      double c_count = 0;
      for (int i = 0; i < data.size(); i++) {
        count = count + 1;
        auto& x = data.at(i).first;
        auto y = std::max(data.at(i).second, 0.);

        double pred_y = 0.0;
        for (auto field : x)
          pred_y += model.at(field.first).second * field.second;

        pred_y = 1. / (1. + exp(-1 * pred_y));
        pred_y = (pred_y > 0.5) ? 1 : 0;
        if (int(pred_y) == int(y)) {
          c_count += 1;
        }
      }
      LOG(INFO) << " accuracy is " << std::to_string(c_count / count);
      google::FlushLogFiles(google::INFO);
      ret.push_back(std::make_pair(c_count, count));
      return ret;
    };
    // Load data
    auto data = TextSourceDataset(input, tg, n_partitions).FlatMap([](const std::string& line) { return LibsvmParse(line); });

    // initialize model
    auto model_partition = std::make_shared<KVDataset>(
        data.MapPartition(initial_gradient)
            .RangeReduceBy([](const std::pair<int, double>& dim) { return dim.first; },
                           [block_dims](const std::pair<int, double>& dim) { return dim.first / block_dims; },
                           [](std::pair<int, double>& dim1, const std::pair<int, double>& dim2) { dim1.second += dim2.second; }, n_partitions2));

    for (int iter = 0; iter < n_iters; ++iter) {
      auto localmodel = model_partition->Broadcast([](const std::pair<int, double>& dim) { return dim.first; }, n_partitions);
      auto gradients =
          data.SharedDataMapPartitionWith(&localmodel, get_gradient)
              .RangeReduceBy([](const std::pair<int, double>& dim) { return dim.first; },
                             [block_dims](const std::pair<int, double>& dim) { return dim.first / block_dims; },
                             [](std::pair<int, double>& dim1, const std::pair<int, double>& dim2) { dim1.second += dim2.second; }, n_partitions2);
      model_partition = std::make_shared<KVDataset>(
          model_partition->SharedDataMapPartitionWith(&gradients, [block_dims](const KVDatasetPartition& lhs, const KVDatasetPartition& rhs) {
            KVDatasetPartition ret;
            CHECK_GE(lhs.size(), rhs.size());
            if (rhs.empty()) {
              return lhs;
            }
            size_t idx = 0;
            for (int i = 0; i < lhs.size(); ++i) {
              if (idx < rhs.size() && rhs.at(idx).first == lhs.at(i).first) {
                ret.push_back(std::make_pair(lhs.at(i).first, lhs.at(i).second + rhs.at(idx++).second));
              } else {
                ret.push_back(lhs.at(i));
              }
            }
            return ret;
          }));
    }

    data.SharedDataMapPartitionWith(std::make_shared<axe::common::Dataset<std::pair<int, double>>>(
                                        model_partition->Broadcast([](const std::pair<int, double>& dim) { return dim.first; }, n_partitions))
                                        .get(),
                                    test_error);

    axe::common::JobDriver::ReversePrintTaskGraph(*tg);
  }
};

int main(int argc, char** argv) {
  axe::common::JobDriver::Run(argc, argv, LogicalRegression());
  return 0;
}
