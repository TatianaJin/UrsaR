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

struct DataObj {
  double GetMemory() const { return vec.capacity() * sizeof(double) + sizeof(DataObj); }

  bool operator<(const DataObj& other) const { return vec.at(0) < vec.at(0); }
  bool operator==(const DataObj& other) const { return vec.at(0) == vec.at(0); }
  friend void operator<<(axe::base::BinStream& bin_stream, const DataObj& v) { bin_stream << v.vec; }
  friend void operator>>(axe::base::BinStream& bin_stream, DataObj& v) { bin_stream >> v.vec; }

  std::vector<double> vec;
};

DatasetPartition<DataObj> LibsvmParse(const std::string& line, int features) {
  DataObj this_obj;
  char* pos;
  std::unique_ptr<char[]> record_ptr(new char[line.size() + 1]);
  strncpy(record_ptr.get(), line.data(), line.size());
  record_ptr.get()[line.size()] = '\0';
  char* tok = strtok_r(record_ptr.get(), " \t:", &pos);

  int i = -1;
  int idx;
  double val;
  int last_id = 0;
  while (tok != NULL) {
    if (i == 0) {
      idx = std::atoi(tok) - 1;
      DCHECK(idx >= 0) << "idx ERROR !" << idx;
      i = 1;
    } else if (i == 1) {
      val = std::atof(tok);
      while (last_id < idx) {
        this_obj.vec.push_back(0);
        last_id++;
      }
      last_id = idx + 1;
      this_obj.vec.push_back(val);
      i = 0;
    } else {
      double x = std::atof(tok);
      i = 0;
    }
    tok = strtok_r(NULL, " \t:", &pos);
  }
  DLOG(INFO) << "features " << features;
  while (last_id < features) {
    this_obj.vec.push_back(0);
    last_id++;
  }
  DatasetPartition<DataObj> ret;
  ret.push_back(this_obj);
  return ret;
}

class DummySourceDataset : public SourceDataset {
 public:
  DummySourceDataset(int parallelism, TaskGraph* task_graph) : SourceDataset(task_graph) {
    SetParallelism(parallelism);
    auto producer = CreateTask("ReadData");
    SetProducer(producer);
    RegisterClosure(producer->GetId(),
                    [id = this->GetId()](TaskContext * tc) { tc->InsertDatasetPartition(id, std::make_shared<DatasetPartition<std::string>>()); });
  }
};

class Kmeans : public Job {
 public:
  void Run(TaskGraph* tg, const std::shared_ptr<Properties>& config) const override {
    auto input = config->GetOrSet("data", "");
    int n_partitions = std::stoi(config->GetOrSet("parallelism", "20"));
    int n_iters = std::stoi(config->GetOrSet("n_iters", "5"));
    int k = std::stoi(config->GetOrSet("k", "10"));
    int features = std::stoi(config->GetOrSet("feature", "0"));

    // Load data
    auto data = TextSourceDataset(input, tg, n_partitions)
                    .FlatMap([features](const std::string& line) { return LibsvmParse(line, features); })
                    .PartitionBy(
                        [n_partitions](const DataObj& vec) {
                          std::random_device rd;
                          std::mt19937 mt(rd());
                          std::uniform_int_distribution<int> dist(0, n_partitions);
                          return dist(mt);
                        },
                        n_partitions);

    auto get_closest = [](const DatasetPartition<DataObj>& data, const DatasetPartition<DataObj>& kmeans) {
      DatasetPartition<std::pair<int, std::pair<int, DataObj>>> ret;
      auto dis = [](const DataObj& x, const DataObj& y) {
        double dis = 0.0;
        for (int i = 0; i < x.vec.size(); i++) {
          dis += (x.vec.at(i) - y.vec.at(i)) * (x.vec.at(i) - y.vec.at(i));
        }
        return dis;
      };

      if (kmeans.size() == 0)
        return ret;

      for (const auto& p : data) {
        int cur_closest = 0;
        double cur_dis = dis(p, kmeans.at(0));
        for (int i = 1; i < kmeans.size(); ++i) {
          double d = dis(p, kmeans.at(i));
          if (d < cur_dis) {
            cur_closest = i;
            cur_dis = d;
          }
        }
        // TODO combine point assignment within shards
        ret.push_back(std::make_pair(cur_closest, std::make_pair(1, p)));
      }
      return ret;
    };

    // initialize model
    auto kmeans = std::make_shared<axe::common::Dataset<DataObj>>(data.PartitionBy(
                                                                          [k](const DataObj& vec) {
                                                                            std::random_device rd;
                                                                            std::mt19937 mt(rd());
                                                                            std::uniform_int_distribution<int> dist(0, 50);
                                                                            return dist(mt);
                                                                          },
                                                                          k)
                                                                      .MapPartition([](const DatasetPartition<DataObj>& data) {
                                                                        DatasetPartition<DataObj> ret;
                                                                        CHECK(data.size() != 0) << " no data error !";
                                                                        ret.push_back(data.at(0));
                                                                        return ret;
                                                                      }));

    for (int iter = 0; iter < n_iters; ++iter) {
      kmeans = std::make_shared<axe::common::Dataset<DataObj>>(
          data.SharedDataMapPartitionWith(
                  std::make_shared<axe::common::Dataset<DataObj>>(kmeans->Broadcast([](const DataObj& mean) { return mean.vec.at(0); }, n_partitions))
                      .get(),
                  get_closest)
              .ReduceBy([](const std::pair<int, std::pair<int, DataObj>>& ele) { return ele.first; },
                        [](std::pair<int, std::pair<int, DataObj>>& agg, const std::pair<int, std::pair<int, DataObj>>& ele) {
                          agg.second.first += ele.second.first;
                          for (int i = 0; i < agg.second.second.vec.size(); i++) {
                            agg.second.second.vec.at(i) += ele.second.second.vec.at(i);
                          }
                        },
                        k)
              .MapPartition([](const auto& partition) {
                DatasetPartition<DataObj> ret;
                for (const auto& par : partition) {
                  auto vec = par.second.second;
                  for (auto& x : vec.vec)
                    x /= par.first;
                  ret.push_back(vec);
                }
                return ret;
              }));
    }
    axe::common::JobDriver::ReversePrintTaskGraph(*tg);
  }
};

int main(int argc, char** argv) {
  axe::common::JobDriver::Run(argc, argv, Kmeans());
  return 0;
}
