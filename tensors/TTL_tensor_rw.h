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
 * @brief  Read a value from a tensor
 *
 * @param tensor A TTL_int_[type]_tensor_t describing the internal tensor.
 * @param x The offset in the x dimension
 * @param y The offset in the y dimension
 * @param z The offset in the z dimension
 *
 * No bounds checking is performed.
 *
 * @return The value read
 */
static inline TTL_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_read_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) tensor,
               const unsigned int x, const unsigned int y, const unsigned int z) {
    return tensor.base[x + (tensor.layout.row_spacing * y) + (tensor.layout.plane_spacing * z)];
}

static inline TTL_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_read_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) tensor,
               const unsigned int x, const unsigned int y) {
    return TTL_read_tensor(tensor, x, y, 0);
}

static inline TTL_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_read_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) tensor,
               const unsigned int x) {
    return TTL_read_tensor(tensor, x, 0, 0);
}

/**
 * @brief  Read a value from a tensor
 *
 * @param sub_tensor A TTL_int_[type]_sub_tensor_t describing the internal tensor.
 * @param x The offset in the x dimension
 * @param y The offset in the y dimension
 * @param z The offset in the z dimension
 *
 * No bounds checking is performed.
 *
 * @return The value read
 */
static inline TTL_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_read_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) sub_tensor,
               const unsigned int x, const unsigned int y, const unsigned int z) {
    return TTL_read_tensor(sub_tensor.tensor, x, y, z);
}

static inline TTL_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_read_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) sub_tensor,
               const unsigned int x, const unsigned int y) {
    return TTL_read_tensor(sub_tensor.tensor, x, y, 0);
}

static inline TTL_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_read_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) sub_tensor,
               const unsigned int x) {
    return TTL_read_tensor(sub_tensor.tensor, x, 0, 0);
}

/**
 * @brief  Write a value from a tensor
 *
 * @param tensor A TTL_int_[type]_tensor_t describing the internal tensor.
 * @param value The value to right
 * @param x The offset in the x dimension
 * @param y The offset in the y dimension
 * @param z The offset in the z dimension
 */
static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_write_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) tensor,
               const TTL_TENSOR_TYPE value, const unsigned int x, const unsigned int y, const unsigned int z) {
    tensor.base[x + (tensor.layout.row_spacing * y) + (tensor.layout.plane_spacing * z)] = value;
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_write_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) tensor,
               const TTL_TENSOR_TYPE value, unsigned int x, const unsigned int y) {
    TTL_write_tensor(tensor, value, x, y, 0);
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_write_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t) tensor,
               const TTL_TENSOR_TYPE value, const unsigned int x) {
    TTL_write_tensor(tensor, value, x, 0, 0);
}

/**
 * @brief  Write a value from a tensor
 *
 * @param sub_tensor A TTL_int_[type]_tensor_t describing the internal tensor.
 * @param value The value to right
 * @param x The offset in the x dimension
 * @param y The offset in the y dimension
 * @param z The offset in the z dimension
 */
static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_write_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) sub_tensor,
               const TTL_TENSOR_TYPE value, const unsigned int x, const unsigned int y, const unsigned int z) {
    TTL_write_tensor(sub_tensor.tensor, value, x, y, z);
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_write_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) sub_tensor,
               const TTL_TENSOR_TYPE value, unsigned int x, const unsigned int y) {
    TTL_write_tensor(sub_tensor.tensor, value, x, y, 0);
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_write_tensor, const __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t) sub_tensor,
               const TTL_TENSOR_TYPE value, const unsigned int x) {
    TTL_write_tensor(sub_tensor.tensor, value, x, 0, 0);
}
