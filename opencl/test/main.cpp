//
// Copyright (c) 2022 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "harness/compat.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness/testHarness.h"
#include "procs.h"

test_definition test_list[] = {
    ADD_TEST(ttl),
};

const int test_num = ARRAY_SIZE(test_list);

int main(int argc, const char *argv[]) {
    return runTestHarness(argc,
                          argv,
                          test_num,
                          test_list,
                          /*forceNoContextCreation = */ false,
                          /*cl_command_queue_properties = */ 0);
}
