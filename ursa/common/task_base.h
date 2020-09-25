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
#include <vector>

#include "common/closure.h"
#include "common/constants.h"
#include "common/resource_predictor.h"

namespace axe {
namespace common {

class TaskBase {
 public:
  TaskBase(TaskIdType id, const TaskNameType& name);

  TaskIdType GetId() const { return id_; }
  Closure GetClosure() const { return closure_; }

  void SetClosure(const Closure& closure) { closure_ = closure; }

  void SetTaskType(ResourceType type) { resource_type_ = type; }

  ResourceType GetTaskType() const { return resource_type_; }

  std::string GetName() const { return name_; }

  void SetResourcePredictor(const ResourcePredictor& predictor) { resource_predictor_ = predictor; }
  const ResourcePredictor& GetResourcePredictor() const { return resource_predictor_; }

 protected:
  TaskIdType id_;
  Closure closure_;
  ResourceType resource_type_;
  TaskNameType name_;
  ResourcePredictor resource_predictor_;
};

}  // namespace common
}  // namespace axe
