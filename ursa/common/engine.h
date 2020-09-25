// Copyright 2020 HDL
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

#include "base/properties.h"
#include "common/closure.h"
#include "common/dataset/dataset_partition.h"
#include "common/dataset/source_dataset.h"
#include "common/job_driver.h"
#include "common/resource_predictor.h"
#include "common/task_graph.h"

using axe::base::BinStream;
using axe::base::Properties;

using axe::common::DatasetPartition;
using axe::common::Job;
using axe::common::ResourcePredictor;
using axe::common::SourceDataset;
using axe::common::TaskContext;
using axe::common::TaskGraph;
using axe::common::TextSourceDataset;

/** A wrapper for application include headers **/
