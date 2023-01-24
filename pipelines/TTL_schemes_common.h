/*
 * TTL_schemes_common.h
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

// This file presumes that the following have been pre included.
// this is not done here for path reasons.
// #include "TTL_core.h"

/**
 * @def TTL_common_buffering_t
 * @brief Common data for description of TTL pipelining.
 *
 * @param ext_base_type Is the type of the pointer that is the internal base. Generall const 'void *' or 'void *'
 * @param ext_tensor_in_type Is the type of the external tensor used for import
 *
 * Contains all the common elements of the pipeline schemes. The more
 * information that can be made common the more opportunity exists for future
 * optimizations and development.
 */
#define TTL_common_buffering_t(ext_base_type, ext_tensor_in_type)                                                 \
    struct {                                                                                                      \
        int index; /** @brief Describes the current buffer index when pipelining. For single 0->1->0, for double  \
                      0->1->0->1... etc */                                                                        \
        TTL_local(ext_base_type) int_base[3]; /** @brief The internal base addresses of the pipelined tiles. Only \
                                                 first 'pipeline_buffers' are valid.*/                            \
                                                                                                                  \
        ext_tensor_in_type ext_tensor_in; /** @brief  The external tensor being input*/                           \
        TTL_ext_tensor_t ext_tensor_out;  /** @brief  The external tensor being output*/                          \
    }

/**
 * @brief Describes a pair of internal Tensors after an operation.
 *
 * The most likely usage is that compute input comes from the imported_to
 * TTL_int_tensor_t and the compute output goes to the to_export_from
 * TTL_int_tensor_t.
 *
 */
typedef struct {
    TTL_int_sub_tensor_t imported_to;     ///< The TTL_int_sub_tensor_t that was most recently imported
    TTL_int_sub_tensor_t to_export_from;  ///< The TTL_int_sub_tensor_t that will be exported next
} TTL_io_tensors_t;

/**
 * @brief Create a TTL_io_tensors_t from a pair of tensors
 *
 * @param imported_to The TTL_intTTL_int_sub_tensor_t_tensor_t that was most recently imported
 * @param to_export_from The TTL_iTTL_int_sub_tensor_tnt_tensor_t that will be exported next
 *
 * @return A TTL_io_tensors_t structure
 */
static inline TTL_io_tensors_t TTL_create_io_tensors(TTL_int_sub_tensor_t imported_to,
                                                     TTL_int_sub_tensor_t to_export_from) {
    TTL_io_tensors_t result;
    result.imported_to = imported_to;
    result.to_export_from = to_export_from;

    return result;
}

static inline int TTL_tensors_empty(TTL_io_tensors_t tensors) {
    return TTL_int_tensor_empty(tensors.imported_to.tensor);
}
