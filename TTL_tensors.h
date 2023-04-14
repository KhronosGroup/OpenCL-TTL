/*
 * TTL_tensors.h
 *
 * Copyright (c) 2023 Mobileye
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "tensors/TTL_ext_tensors.h"
#include "tensors/TTL_int_tensors.h"


#ifndef FROM_C
void TTL_fetch_tensor(__local TTL_ext_tensor_t *local_ext_tensor, const __global TTL_ext_tensor_t *global_ext_tensor) {
    event_t event = async_work_group_copy(
        (__local uchar *)local_ext_tensor, (const __global uchar *)global_ext_tensor, sizeof(*local_ext_tensor), 0);
    wait_group_events(1, &event);
}
#endif