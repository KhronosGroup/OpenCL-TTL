/*
 * TTL_ext_tensors.h
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

#define TENSOR_LOCATION int_
#define TENSOR_ADDRESS TTL_local
// INT TENSORS START
#define TYPES_INCLUDE_FILE "tensors/TTL_int_ext_typed_tensors.h"
#include "../TTL_create_types.h"
// INT TENSORS END
#undef TYPES_INCLUDE_FILE
#undef TENSOR_LOCATION
#undef TENSOR_ADDRESS

/* Make void the default unnamed type */
typedef TTL_int_void_tensor_t TTL_int_tensor_t;
typedef TTL_const_int_void_tensor_t TTL_const_int_tensor_t;
typedef TTL_int_void_sub_tensor_t TTL_int_sub_tensor_t;
typedef TTL_const_int_void_sub_tensor_t TTL_const_int_sub_tensor_t;
