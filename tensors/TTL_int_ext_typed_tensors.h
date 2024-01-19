/*
 * TTL_int_tensors.h
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

#include "../TTL_types.h"
#include "TTL_tensors_common.h"

/**
 * @file
 * 
 * When including this file the follow must be defined
 * 
 * TENSOR_LOCATION ext_ or TENSOR_LOCATION
 * TENSOR_ADDRESS TENSOR_ADDRESS or TTL_global
 * TTL_TENSOR_TYPE void, uchar, uchar2 etc etc.
*/

/******************************************************
 * Tensors const and non-const
 *****************************************************/

/**
 * @brief const and non-const tensors in the appropriate address space
 *
 * @see __TTL_typedef_tensor_t
 *
 * Use of macros to ensure that structs are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_int_tensor_t to be safely case to a TTL_const_int_tensor_t.
 *
 * Do not add a cast for TTL_const_int_tensor_t to TTL_int_tensor_t because that is not safe.
 */
__TTL_typedef_tensor_t(TENSOR_ADDRESS, , TENSOR_LOCATION, TTL_TENSOR_TYPE, );             ///< @brief create TTL_int_tensor_t
__TTL_typedef_tensor_t(TENSOR_ADDRESS, const_, TENSOR_LOCATION, TTL_TENSOR_TYPE, const);  ///< @brief create TTL_const_int_tensor_t

/**
 * @brief const and non-const tensor creation functions.
 *
 * @see __TTL_create_tensor_impl
 *
 * Use of macros to ensure that functions are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_int_tensor_t to be safely case to a TTL_const_int_tensor_t.
 */
__TTL_create_tensor_impl(TENSOR_ADDRESS, , TENSOR_LOCATION, TTL_TENSOR_TYPE, );             ///< @brief create TTL_create_int_tensor_impl
__TTL_create_tensor_impl(TENSOR_ADDRESS, const_, TENSOR_LOCATION, TTL_TENSOR_TYPE, const);  ///< @brief create TTL_create_const_int_tensor_impl

__TTL_create_create_tensor_functions(TENSOR_ADDRESS, , TENSOR_LOCATION, TTL_TENSOR_TYPE, , );
__TTL_create_create_tensor_functions(TENSOR_ADDRESS, const_, TENSOR_LOCATION, TTL_TENSOR_TYPE, , const);

/******************************************************
 * Sub Tensors const and non-const
 *****************************************************/
/**
 * @brief const and non-const sub tensors in the appropriate address space
 *
 * @see __TTL_typedef_sub_tensor_t
 *
 * Use of macros to ensure that structs are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_int_tensor_t to be safely case to a TTL_const_int_tensor_t.
 *
 * Do not add a cast for TTL_const_int_tensor_t to TTL_int_tensor_t because that is not safe.
 */
__TTL_typedef_sub_tensor_t(TTL_gobal, , TENSOR_LOCATION, TTL_TENSOR_TYPE, );  ///< @brief create TTL_int_sub tensor_t
__TTL_typedef_sub_tensor_t(TTL_gobal, const_, TENSOR_LOCATION, TTL_TENSOR_TYPE,
                           const);  ///< @brief create TTL_const_int_sub tensor_t

/**
 * @brief const and non-const sub tensor creation functions.
 *
 * @see __TTL_create_sub_tensor_impl
 *
 * Use of macros to ensure that functions are EXACTLY the same, along for the compiler to remove duplication
 * and potentially a TTL_int_tensor_t to be safely case to a TTL_const_int_tensor_t.
 */
__TTL_create_sub_tensor_impl(TENSOR_ADDRESS, , TENSOR_LOCATION, TTL_TENSOR_TYPE, );  ///< @brief create TTL_create_int_sub_tensor_impl
__TTL_create_sub_tensor_impl(TENSOR_ADDRESS, const_, TENSOR_LOCATION, TTL_TENSOR_TYPE,
                             const);  ///< @brief create TTL_create_int_sub_tensor_impl

__TTL_create_create_sub_tensor_functions(TENSOR_ADDRESS, , TENSOR_LOCATION, TTL_TENSOR_TYPE, sub_, );
__TTL_create_create_sub_tensor_functions(TENSOR_ADDRESS, const_, TENSOR_LOCATION, TTL_TENSOR_TYPE, sub_, const);