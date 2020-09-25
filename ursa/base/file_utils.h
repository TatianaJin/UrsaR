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

namespace axe {
namespace base {

void WriteToFile(const std::string& file_name, const char* bytes, uint32_t size);

bool Exists(const std::string& file_name);

/** Unix platform only **/
int ExistsExecutable(const std::string& file_name);

/** Unix platform only
 * Make the file executable by owner
 */
int MakeExecutable(const std::string& file_name);

std::string Strip(const std::string& str);

}  // namespace base
}  // namespace axe
