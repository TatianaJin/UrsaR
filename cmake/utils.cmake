# Copyright 2018 H-AXE
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Short command for caching cmake variable
# Usage:
#   cache_variable(<var> <value>:[arg1, arg2, ...])
function(cache_variable var)
    set(__list "")
    foreach(value ${ARGN})
        list(APPEND __list ${value})
    endforeach()
    set(${var} ${__list} CACHE INTERNAL "")
endfunction()
