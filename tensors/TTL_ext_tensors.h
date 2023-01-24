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

#include "../TTL_types.h"
#include "TTL_tensors_common.h"

/******************************************************
 * External Tensors const and non-const
 *****************************************************/

/**
 * @brief const and non-const external tensors in the global address space
 *
 * @see __TTL_typedef_tensor_t
 *
 * Use of macros to ensure that structs are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_ext_tensor_t to be safely case to a TTL_const_ext_tensor_t.
 *
 * Do not add a cast for TTL_const_ext_tensor_t to TTL_ext_tensor_t because that is not safe.
 */
__TTL_typedef_tensor_t(TTL_global, , ext_, void, );             ///< @brief create TTL_ext_tensor_t
__TTL_typedef_tensor_t(TTL_global, const_, ext_, void, const);  ///< @brief create TTL_const_ext_tensor_t

/* Make void the default unnamed type */
typedef TTL_ext_void_tensor_t TTL_ext_tensor_t;
typedef TTL_const_ext_void_tensor_t TTL_const_ext_tensor_t;

/**
 * @brief const and non-const external tensor creation functions.
 *
 * @see __TTL_create_tensor_impl
 *
 * Use of macros to ensure that functions are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_ext_tensor_t to be safely case to a TTL_const_ext_tensor_t.
 */
__TTL_create_tensor_impl(TTL_global, , ext_, void, );             ///< @brief create TTL_create_ext_tensor_impl
__TTL_create_tensor_impl(TTL_global, const_, ext_, void, const);  ///< @brief create TTL_create_const_ext_tensor_impl

__TTL_create_create_tensor_functions(TTL_global, , ext_, void, , );
__TTL_create_create_tensor_functions(TTL_global, const_, ext_, void, , const);

/******************************************************
 * External Sub Tensors const and non-const
 *****************************************************/
/**
 * @brief const and non-const external sub tensors in the global address space
 *
 * @see __TTL_typedef_sub_tensor_t
 *
 * Use of macros to ensure that structs are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_ext_tensor_t to be safely case to a TTL_const_ext_tensor_t.
 *
 * Do not add a cast for TTL_const_ext_tensor_t to TTL_ext_tensor_t because that is not safe.
 */
__TTL_typedef_sub_tensor_t(TTL_gobal, , ext_, void, );  ///< @brief create TTL_ext_sub tensor_t
__TTL_typedef_sub_tensor_t(TTL_gobal, const_, ext_, void,
                           const);  ///< @brief create TTL_const_ext_sub tensor_t

/* Make void the default unnamed type */
typedef TTL_ext_void_sub_tensor_t TTL_ext_sub_tensor_t;
typedef TTL_const_ext_void_sub_tensor_t TTL_const_ext_sub_tensor_t;

/**
 * @brief const and non-const external sub tensor creation functions.
 *
 * @see __TTL_create_sub_tensor_impl
 *
 * Use of macros to ensure that functions are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_ext_tensor_t to be safely case to a TTL_const_ext_tensor_t.
 */
__TTL_create_sub_tensor_impl(TTL_global, , ext_, void, );  ///< @brief create TTL_create_ext_sub_tensor_impl
__TTL_create_sub_tensor_impl(TTL_global, const_, ext_, void,
                             const);  ///< @brief create TTL_create_ext_sub_tensor_impl

__TTL_create_create_sub_tensor_functions(TTL_global, , ext_, void, sub_, );
__TTL_create_create_sub_tensor_functions(TTL_global, const_, ext_, void, sub_, const);
