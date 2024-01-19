/*
 * TTL_tensors_common.h
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

#include "../TTL_macros.h"
#include "../TTL_types.h"

/**
 * @brief  Import the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor A TTL_int_tensor_t describing the external tensor.
 * Complete description of what not how here.
 *
 * @return No return value
 */
static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_import, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) internal_tensor,
               const __TTL_tensor_name(TTL_, const_, ext_, TTL_TENSOR_TYPE, , _t) external_tensor, TTL_event_t *event) {
    return TTL_import_base(
        *TTL_to_void_tensor(&internal_tensor), *TTL_to_void_tensor(&external_tensor), event __TTL_TRACE_LINE);
}

/**
 * @brief  Import the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor A TTL_int_tensor_t describing the external tensor.
 * Complete description of what not how here.
 *
 * @return No return value
 */
static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_blocking_import, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) internal_tensor,
               const __TTL_tensor_name(TTL_, const_, ext_, TTL_TENSOR_TYPE, , _t) external_tensor) {
    TTL_blocking_import_base(*TTL_to_void_tensor(&internal_tensor), *TTL_to_void_tensor(&external_tensor) __TTL_TRACE_LINE);
}

/**
 * @brief Implementation of TTL_import_sub_tensor
 *
 * @param internal_sub_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param const_external_tensor A TTL_const_ext_tensor_t describing the external tensor.
 * @param event A TTL_event_t type to allow detection of import completion.
 *
 * @see TTL_import for full API and parameter information
 */
static inline void __attribute__((overloadable)) __TTL_TRACE_FN(
    TTL_import_sub_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) internal_sub_tensor,
    const __TTL_tensor_name(TTL_, const_, ext_, TTL_TENSOR_TYPE, , _t) const_external_tensor, TTL_event_t *event) {
    TTL_local(void *) dst_address;
    TTL_global(void *) src_address;

    const TTL_shape_t import_shape = TTL_import_pre_fill(*TTL_to_void_sub_tensor(&internal_sub_tensor), *TTL_to_void_tensor(&const_external_tensor), &dst_address, &src_address);

    const TTL_int_tensor_t import_int_tensor = TTL_create_int_tensor(
        dst_address, import_shape, internal_sub_tensor.tensor.layout, internal_sub_tensor.tensor.elem_size);

    const TTL_const_ext_tensor_t import_ext_tensor = TTL_create_const_ext_tensor(
        src_address, import_shape, const_external_tensor.layout, TTL_create_offset(), const_external_tensor.elem_size);

    TTL_import(import_int_tensor, import_ext_tensor, event __TTL_TRACE_LINE);
}

/**
 * @brief  Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor A TTL_int_tensor_t describing the external tensor.
 * Complete description of what not how here.
 *
 * @return No return value
 */
static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_export, const __TTL_tensor_name(TTL_, const_, int_, TTL_TENSOR_TYPE, , _t) internal_tensor,
               const __TTL_tensor_name(TTL_, , ext_, TTL_TENSOR_TYPE, , _t) external_tensor, TTL_event_t *event) {
    return TTL_export_base(
        *TTL_to_void_tensor(&internal_tensor), *TTL_to_void_tensor(&external_tensor), event __TTL_TRACE_LINE);
}

/**
 * @brief  Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor A TTL_int_tensor_t describing the external tensor.
 * Complete description of what not how here.
 *
 * @return No return value
 */
static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_blocking_export, const __TTL_tensor_name(TTL_, const_, int_, TTL_TENSOR_TYPE, , _t) internal_tensor,
               const __TTL_tensor_name(TTL_, , ext_, TTL_TENSOR_TYPE, , _t) external_tensor) {
    TTL_blocking_export_base(*TTL_to_void_tensor(&internal_tensor), *TTL_to_void_tensor(&external_tensor) __TTL_TRACE_LINE);
}