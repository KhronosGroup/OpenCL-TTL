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

for type in char uchar short ushort int uint long ulong; do
  for compute in cross square copy; do

    echo Compute $compute with Tensor of type $type
    clang -O0 -g -D TTL_TARGET=c -D TEST_TENSOR_TYPE=$type -D COMPUTE=$compute.h -D CL_TARGET_OPENCL_VERSION=300 -I $TTL_INCLUDE_PATH -I $OPEN_CL_INCLUDE_PATH -o TTL_sample_overlap TTL_sample_runner.cpp -lOpenCL -lstdc++
    ./TTL_sample_overlap

  done
done
