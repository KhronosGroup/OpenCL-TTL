#!/bin/bash

# test_all_types.h
#
# Copyright (c) 2023 Mobileye
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

if [ -z "${OPEN_CL_INCLUDE_PATH}" ]; then
    TTL_OPEN_CL_INCLUDE_PATH=""
else
    TTL_OPEN_CL_INCLUDE_PATH="-I $OPEN_CL_INCLUDE_PATH"
fi

# Output:
# VAR is set to some string


for types in char,%c uchar,%c short,%d ushort,%u int,%d uint,%u long,%ld ulong,%lu; do
  IFS=","; set -- $types
  type=$1
  type_specifier=$2

  for compute in cross square copy; do
    echo Compute $compute with tensor of type $type compute of $compute
    clang -O0 -g -D TTL_TARGET=c -D TEST_TENSOR_TYPE=$type -D TEST_TENSOR_TYPE_SPECIFIER="\"$type_specifier\"" -D COMPUTE=$compute.h -D CL_TARGET_OPENCL_VERSION=300 -I $TTL_INCLUDE_PATH $TTL_OPEN_CL_INCLUDE_PATH -o TTL_sample_overlap TTL_sample_runner.cpp -lOpenCL -lstdc++
    ./TTL_sample_overlap

  done
done
