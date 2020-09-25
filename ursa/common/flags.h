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

#include "gflags/gflags.h"

DECLARE_string(master_hostname);
DECLARE_int32(master_port);
DECLARE_int32(client_listener_port);

DECLARE_string(worker_hostname);
DECLARE_int32(worker_port);

DECLARE_int32(job_id);

DECLARE_string(jp_file);
DECLARE_string(config_file);
DECLARE_string(master_husky_scratch_dir);
DECLARE_string(worker_husky_scratch_dir);

DECLARE_bool(enable_hdfs);
DECLARE_string(hdfs_namenode);
DECLARE_int32(hdfs_port);

DECLARE_int32(worker_cpu_cores);
DECLARE_int32(worker_mem_gb);
DECLARE_int32(worker_net_mb);
DECLARE_int32(worker_disk_mb);

DECLARE_int32(net_schedule_factor);
DECLARE_int32(net_small_threshold);
DECLARE_double(cpu_schedule_factor);
DECLARE_double(network_schedule_factor);
DECLARE_double(disk_schedule_factor);
DECLARE_double(peak_memory_factor);

DECLARE_string(jm_mode);
DECLARE_string(task_queue_mode);
DECLARE_string(task_queue_manager_mode);
DECLARE_string(scheduler_mode);
DECLARE_string(scheduling_policy);
DECLARE_string(job_admission_queue_type);
DECLARE_string(peak_memory_estimator_type);

DECLARE_int32(schedule_time_interval_ms);
DECLARE_double(load_balance_cpu_threshold);
DECLARE_double(load_balance_network_threshold);
DECLARE_double(load_balance_disk_threshold);
DECLARE_int32(overselling_factor);
DECLARE_bool(prioritize_job);
DECLARE_bool(set_nice_value);
DECLARE_string(priority_server_addr);
DECLARE_double(cpu_process_rate);
DECLARE_double(net_bandwidth);
DECLARE_double(subgraph_max_memory);
DECLARE_int32(max_subgraph_per_worker);
DECLARE_int32(max_running_jobs);
DECLARE_double(ept);

DECLARE_double(aon_threshold);

DECLARE_string(resource_estimator);

// for control experiments
DECLARE_bool(exclude_net_ept);
DECLARE_bool(enable_message_size_report);

DECLARE_bool(singleton_yarn);
DECLARE_string(worker_addrs);
DECLARE_string(jm_log_prefix);
