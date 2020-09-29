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

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "glog/logging.h"

#include "base/bin_stream.h"
#include "common/dataset/abstract_data.h"
#include "common/dataset/abstract_dataset.h"
#include "common/dataset/dataset_partition.h"
#include "common/task.h"
#include "common/task_context.h"
#include "common/task_graph.h"

namespace axe {
namespace common {

using base::BinStream;

template <typename Val>
class Dataset : public AbstractDataset {
 public:
  inline static Dataset<Val> Create(const std::shared_ptr<Task>& creator, TaskGraph* task_graph, int parallelism = 10) {
    return Dataset<Val>(creator, task_graph, parallelism);
  }

  /* APIs */

  template <typename Lambda>
  void ApplyRead(Lambda lambda) {
    SanityCheck();
    auto task = CreateTask("ApplyRead");
    RegisterClosure(task->GetId(), [ lambda, id = this->id_ ](TaskContext * tc) { lambda(*(tc->GetDatasetPartition<Val>(id))); });
    ReadBy(task);
  }

  template <typename Lambda>
  auto MapPartition(Lambda lambda) {
    SanityCheck();
    auto task = CreateTask("MapPartition");
    using ret_type = typename decltype(lambda(DatasetPartition<Val>()))::value_type;
    auto ret = Dataset<ret_type>::Create(task, task_graph_, parallelism_);
    RegisterClosure(task->GetId(), [ lambda, ret = ret.GetId(), id = this->id_ ](TaskContext * tc) {
      auto res_data = std::make_shared<DatasetPartition<ret_type>>(lambda(*(tc->GetDatasetPartition<Val>(id))));
      tc->InsertDatasetPartition(ret, res_data);
    });
    ReadBy(task);
    return ret;
  }

  template <typename Lambda, typename OVal>
  auto MapPartitionWith(Dataset<OVal>* other, Lambda lambda) {
    SanityCheck();
    auto task = CreateTask("MapPartitionWith");
    using ret_type = typename decltype(lambda(DatasetPartition<Val>(), DatasetPartition<OVal>()))::value_type;
    auto ret = Dataset<ret_type>::Create(task, task_graph_, parallelism_);
    RegisterClosure(task->GetId(), [ lambda, ret = ret.GetId(), id = id_, oid = other->GetId() ](TaskContext * tc) {
      auto lhs_data = tc->GetDatasetPartition<Val>(id);
      auto rhs_data = tc->GetDatasetPartition<OVal>(oid);

      auto res_data = std::make_shared<DatasetPartition<ret_type>>(lambda(*lhs_data, *rhs_data));
      tc->InsertDatasetPartition(ret, res_data);
    });
    ReadBy(task);
    other->ReadBy(task);
    return ret;
  }

  template <typename Lambda, typename OVal>
  auto SharedDataMapPartitionWith(Dataset<OVal>* other, Lambda lambda) {
    SanityCheck();
    auto task = CreateTask("SharedMapPartitionWith");
    using ret_type = typename decltype(lambda(DatasetPartition<Val>(), DatasetPartition<OVal>()))::value_type;
    auto ret = Dataset<ret_type>::Create(task, task_graph_, parallelism_);
    RegisterClosure(task->GetId(), [ lambda, ret = ret.GetId(), id = id_, oid = other->GetId() ](TaskContext * tc) {
      auto lhs_data = tc->GetDatasetPartition<Val>(id);
      auto rhs_data = tc->GetDatasetPartition<OVal>(oid);
      auto res_data = std::make_shared<DatasetPartition<ret_type>>(lambda(*lhs_data, *rhs_data));
      tc->InsertDatasetPartition(ret, res_data);
    });
    other->ReadBy(task);
    task->ReadData(id_);
    return ret;
  }

  template <typename Lambda>
  void UpdatePartition(Lambda lambda) {
    SanityCheck();
    auto task = CreateTask("UpdatePartition");
    RegisterClosure(task->GetId(), [&, id = id_ ](TaskContext * tc) {
      auto data = tc->GetMutableDatasetPartition<Val>(id);
      lambda(*data);
    });
    WriteBy(task);
  }

  template <typename Lambda, typename OVal>
  void UpdatePartitionWith(Dataset<OVal>* other, Lambda lambda) {
    SanityCheck();
    auto task = CreateTask("UpdatePartitionWith");
    RegisterClosure(task->GetId(), [ lambda, id = id_, oid = other->GetId() ](TaskContext * tc) {
      auto lhs_data = tc->GetMutableDatasetPartition<Val>(id);
      auto rhs_data = tc->GetDatasetPartition<OVal>(oid);
      lambda(*lhs_data, *rhs_data);
    });
    WriteBy(task);
    other->ReadBy(task);
  }

  /**
   * Partition dataset by key. The resulting dataset has each key in and only in one partition, but not sorted within partition.
   */
  template <typename KeySelector>
  auto PartitionBy(KeySelector key_selector, int num_partitions = 0) {
    SanityCheck();
    if (num_partitions == 0) {
      num_partitions = parallelism_;
    }

    auto serialize = CreateTask("PartitionBy-serialize");
    auto net_task = CreateTask("PartitionBy-shuffle", num_partitions, NetWork);
    auto deserialize = CreateTask("PartitionBy-deserialize", num_partitions);

    auto message = Dataset<BinStream>::Create(serialize, task_graph_, parallelism_);
    auto shuffled = Dataset<BinStream>::Create(net_task, task_graph_, num_partitions);
    auto ret = Dataset<Val>::Create(deserialize, task_graph_, num_partitions);

    RegisterClosure(serialize->GetId(), [ key_selector, num_partitions, msg_id = message.GetId(), id = id_ ](TaskContext * tc) {
      auto this_partition = tc->GetDatasetPartition<Val>(id);
      auto msg = std::make_shared<DatasetPartition<std::shared_ptr<BinStream>>>(
          CreateMessagePartition(num_partitions, msg_id, tc->GetShardId(), tc->GetInstanceId()));
      for (auto& record : *this_partition) {
        *(msg->at(hash(key_selector(record)) % num_partitions)) << record;
      }
      tc->InsertDatasetPartition(msg_id, msg);
    });

    RegisterClosure(deserialize->GetId(), [ msg_id = shuffled.GetId(), ret_id = ret.GetId(), key_selector ](TaskContext * tc) {
      auto msg = tc->GetDatasetPartition<std::shared_ptr<BinStream>>(msg_id);
      auto data = std::make_shared<DatasetPartition<Val>>();
      for (auto& binstream_ptr : *msg) {
        while (binstream_ptr->size() > 0) {
          Val val;
          *binstream_ptr >> val;
          data->push_back(val);
        }
      }
      tc->InsertDatasetPartition(ret_id, data);
    });

    ReadBy(serialize);
    message.ReadBy(net_task);
    deserialize->ReadData(shuffled.GetId());
    net_task->AggregateThen(deserialize);
    return ret;
  }

  template <typename Lambda>
  auto LocalAggregate(Lambda lambda, int partitions) {
    SanityCheck();

    auto aggregate = CreateTask("LocalAggregate");
    auto ret = Dataset<Val>::Create(aggregate, task_graph_, partitions);

    RegisterClosure(aggregate->GetId(), [ lambda = lambda, ret_id = ret.GetId(), id = id_ ](TaskContext * tc) {
      auto this_dataset = tc->GetDataset<Val>(id);

      auto it = this_dataset.begin();
      auto res_data = std::make_shared<DatasetPartition<Val>>(it->second);
      for (++it; it != this_dataset->end(); ++it) {
        res_data = lambda(res_data, it->second);
      }
      tc->InsertDatasetPartition(ret_id, res_data);
    });
    LocalAggregateBy(aggregate);
    return ret;
  }

  template <typename Key>
  auto Broadcast(Key key, int num_partitions = 0, bool use_sort = 1) {
    SanityCheck();
    if (num_partitions == 0) {
      num_partitions = parallelism_;
    }

    auto serialize = CreateTask("Broadcast-serialize");
    auto net_task = CreateTask("Broadcast-shuffle", num_partitions, NetWork);
    auto deserialize = CreateTask("Broadcast-deserialize", num_partitions);
    auto get_processlevel_data = CreateTask("Broadcast-getprocessleveldata", num_partitions);

    auto message = Dataset<BinStream>::Create(serialize, task_graph_, parallelism_);
    auto shuffled = Dataset<BinStream>::Create(net_task, task_graph_, num_partitions);
    auto deserialize_ret = Dataset<Val>::Create(deserialize, task_graph_, num_partitions);
    auto ret = Dataset<Val>::Create(get_processlevel_data, task_graph_, num_partitions);

    RegisterClosure(serialize->GetId(), [ num_partitions, msg_id = message.GetId(), id = id_ ](TaskContext * tc) {
      auto this_partition = tc->GetDatasetPartition<Val>(id);
      auto msg = std::make_shared<DatasetPartition<std::shared_ptr<BinStream>>>(
          CreateMessagePartition(num_partitions, msg_id, tc->GetShardId(), tc->GetInstanceId()));
      for (auto& record : *this_partition) {
        (*msg->at(0)) << record;
      }
      tc->InsertDatasetPartition(msg_id, msg);
    });

    RegisterClosure(deserialize->GetId(), [ use_sort, key, msg_id = shuffled.GetId(), ret_id = deserialize_ret.GetId() ](TaskContext * tc) {

      auto msg = tc->GetDatasetPartition<std::shared_ptr<BinStream>>(msg_id);
      DatasetPartition<Val> data;
      for (auto& binstream_ptr : *msg) {
        if (binstream_ptr->size() == 0) {
          LOG(WARNING) << "fetched empty binstream";
          google::FlushLogFiles(google::WARNING);
        }
        while (binstream_ptr->size() > 0) {
          Val val;
          *binstream_ptr >> val;
          data.push_back(val);
        }
      }

      if (data.empty()) {
        DLOG(INFO) << "No data received for current shard";
        tc->InsertDatasetPartition(ret_id, std::make_shared<DatasetPartition<Val>>(data));
        return;
      }

      // sort by key
      if (use_sort)
        std::sort(data.begin(), data.end(), [key](const Val& a, const Val& b) { return key(a) < key(b); });

      tc->InsertProcessLevelData(ret_id, std::make_shared<DatasetPartition<Val>>(data));
    });

    RegisterClosure(get_processlevel_data->GetId(), [ des_id = deserialize_ret.GetId(), ret_id = ret.GetId() ](TaskContext * tc) {
      auto des_ret = tc->GetProcessLevelData(des_id);
      tc->InsertDatasetPartition<Val>(ret_id, std::dynamic_pointer_cast<DatasetPartition<Val>>(des_ret));
    });
    ReadBy(serialize);
    message.BroadcastBy(net_task);
    deserialize->ReadData(shuffled.GetId());
    net_task->AggregateThen(deserialize);
    get_processlevel_data->ReadData(deserialize_ret.GetId());
    deserialize->Then(get_processlevel_data);
    return ret;
  }

  template <typename Key, typename KeyHash, typename Combiner = std::function<void(Val&, const Val&)>>
  auto RangeReduceBy(Key key, KeyHash key_hash, Combiner combiner, int num_partitions = 0) {
    SanityCheck();
    if (num_partitions == 0) {
      num_partitions = parallelism_;
    }

    auto serialize = CreateTask("RangeReduceBy-serialize");
    auto net_task = CreateTask("RangeReduceBy-shuffle", num_partitions, NetWork);
    auto deserialize = CreateTask("RangeReduceBy-deserialize", num_partitions);

    auto message = Dataset<BinStream>::Create(serialize, task_graph_, parallelism_);
    auto shuffled = Dataset<BinStream>::Create(net_task, task_graph_, num_partitions);
    auto ret = Dataset<Val>::Create(deserialize, task_graph_, num_partitions);

    RegisterClosure(serialize->GetId(), [ key_hash, num_partitions, msg_id = message.GetId(), id = id_ ](TaskContext * tc) {
      auto this_partition = tc->GetDatasetPartition<Val>(id);
      auto msg = std::make_shared<DatasetPartition<std::shared_ptr<BinStream>>>(
          CreateMessagePartition(num_partitions, msg_id, tc->GetShardId(), tc->GetInstanceId()));
      for (auto& record : *this_partition) {
        *(msg->at(key_hash(record) % num_partitions)) << record;
      }

      tc->InsertDatasetPartition(msg_id, msg);
    });

    RegisterClosure(deserialize->GetId(), [ msg_id = shuffled.GetId(), ret_id = ret.GetId(), key_hash, key, combiner ](TaskContext * tc) {
      auto time0 = std::chrono::steady_clock::now();
      auto msg = tc->GetDatasetPartition<std::shared_ptr<BinStream>>(msg_id);
      DatasetPartition<Val> data;
      // Deserialize
      for (auto& binstream_ptr : *msg) {
        while (binstream_ptr->size() > 0) {
          Val val;
          *binstream_ptr >> val;
          data.push_back(val);
        }
      }

      if (data.empty()) {
        DLOG(INFO) << "No data received for current shard";
        tc->InsertDatasetPartition(ret_id, std::make_shared<DatasetPartition<Val>>(data));
        return;
      }

      // Reduce if not empty
      std::sort(data.begin(), data.end(), [key](const Val& a, const Val& b) { return key(a) < key(b); });
      auto current_key = key(data.front());
      size_t current_idx = 0, count = 0;
      for (size_t i = 1; i < data.size(); ++i) {
        auto this_key = key(data[i]);
        if (this_key == current_key) {
          combiner(data[current_idx], data[i]);
        } else {
          data.at(count++) = data.at(current_idx);
          current_idx = i;
          current_key = this_key;
        }
      }
      data.at(count++) = data.at(current_idx);
      data.resize(count);  // TODO(tatiana): free the memory unused? may cause reallocation
      tc->InsertDatasetPartition(ret_id, std::make_shared<DatasetPartition<Val>>(data));

      auto time1 = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
      LOG(INFO) << "rangereduce time " << elapsed;
    });

    ReadBy(serialize);
    message.ReadBy(net_task);
    deserialize->ReadData(shuffled.GetId());
    net_task->AggregateThen(deserialize);
    return ret;
  }

  template <typename KeySelector, typename Combiner = std::function<void(Val&, const Val&)>>
  auto ReduceBy(KeySelector key_selector, Combiner combiner, int num_partitions = 0) {
    SanityCheck();
    if (num_partitions == 0) {
      num_partitions = parallelism_;
    }

    auto serialize = CreateTask("ReduceBy-serialize");
    auto net_task = CreateTask("ReduceBy-shuffle", num_partitions, NetWork);
    auto deserialize = CreateTask("ReduceBy-deserialize", num_partitions);

    auto message = Dataset<BinStream>::Create(serialize, task_graph_, parallelism_);
    auto shuffled = Dataset<BinStream>::Create(net_task, task_graph_, num_partitions);
    auto ret = Dataset<Val>::Create(deserialize, task_graph_, num_partitions);

    RegisterClosure(serialize->GetId(), [ key_selector, &combiner, num_partitions, msg_id = message.GetId(), id = id_ ](TaskContext * tc) {
      auto this_partition = tc->GetDatasetPartition<Val>(id);
      std::vector<std::vector<Val>> local_buffer(num_partitions);
      auto msg = std::make_shared<DatasetPartition<std::shared_ptr<BinStream>>>(
          CreateMessagePartition(num_partitions, msg_id, tc->GetShardId(), tc->GetInstanceId()));
      for (auto& record : *this_partition) {
        local_buffer.at(hash(key_selector(record)) % num_partitions).push_back(record);
      }

      // Combine & serialize
      for (auto iter = local_buffer.begin(); iter != local_buffer.end(); ++iter) {
        if (iter->empty()) {
          continue;
        }
        std::sort(iter->begin(), iter->end(), [key_selector](const Val& a, const Val& b) { return key_selector(a) < key_selector(b); });
        auto current_key = key_selector(iter->front());
        size_t current_idx = 0;
        for (size_t i = 1; i < iter->size(); ++i) {
          auto this_key = key_selector((*iter)[i]);
          if (this_key == current_key) {
            combiner((*iter)[current_idx], (*iter)[i]);
          } else {
            *(msg->at(hash(current_key) % num_partitions)) << (*iter)[current_idx];
            current_idx = i;
            current_key = this_key;
          }
        }
        *(msg->at(hash(current_key) % num_partitions)) << (*iter)[current_idx];
      }

      tc->InsertDatasetPartition(msg_id, msg);
    });

    RegisterClosure(deserialize->GetId(), [ msg_id = shuffled.GetId(), ret_id = ret.GetId(), key_selector, &combiner ](TaskContext * tc) {
      auto msg = tc->GetDatasetPartition<std::shared_ptr<BinStream>>(msg_id);
      DatasetPartition<Val> data;
      // Deserialize
      for (auto& binstream_ptr : *msg) {
        while (binstream_ptr->size() > 0) {
          Val val;
          *binstream_ptr >> val;
          data.push_back(val);
        }
      }

      if (data.empty()) {
        DLOG(INFO) << "No data received for current shard";
        tc->InsertDatasetPartition(ret_id, std::make_shared<DatasetPartition<Val>>(data));
        return;
      }

      // Reduce if not empty
      std::sort(data.begin(), data.end(), [key_selector, &combiner](const Val& a, const Val& b) { return key_selector(a) < key_selector(b); });
      auto current_key = key_selector(data.front());
      size_t current_idx = 0, count = 0;
      for (size_t i = 1; i < data.size(); ++i) {
        auto this_key = key_selector(data[i]);
        if (this_key == current_key) {
          combiner(data[current_idx], data[i]);
        } else {
          data.at(count++) = data.at(current_idx);
          current_idx = i;
          current_key = this_key;
        }
      }

      data.at(count++) = data.at(current_idx);
      data.resize(count);  // TODO(tatiana): free the memory unused? may cause reallocation
      DLOG(INFO) << "ReduceBy: reducer has " << count << " records";
      tc->InsertDatasetPartition(ret_id, std::make_shared<DatasetPartition<Val>>(data));
    });

    ReadBy(serialize);
    message.ReadBy(net_task);
    deserialize->ReadData(shuffled.GetId());
    net_task->AggregateThen(deserialize);
    return ret;
  }

 protected:
  Dataset<Val>(TaskGraph* tg) : AbstractDataset(tg) {}
  Dataset<Val>(const std::shared_ptr<Task>& producer, TaskGraph* task_graph, int parallelism = 10) : AbstractDataset(producer, task_graph) {
    DCHECK_EQ(parallelism, producer->GetParallelism());
  }
};

}  // namespace common
}  // namespace axe
