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

// This file presumes that the following have been pre included.
// this is not done here for path reasons.
// #include "TTL_core.h"
#ifndef DEFINING_TTL_SCHEMES_COMMON
#pragma push_macro("TYPES_INCLUDE_FILE")
#undef TYPES_INCLUDE_FILE
#define TYPES_INCLUDE_FILE "pipelines/TTL_schemes_common.h"
#define DEFINING_TTL_SCHEMES_COMMON BOB
#include "../TTL_create_types.h"
#pragma once
#undef DEFINING_TTL_SCHEMES_COMMON
#pragma pop_macro("TYPES_INCLUDE_FILE")
#else
/**
 * @def TTL_common_buffering_t
 * @brief Common data for description of TTL pipelining.
 *
 * @param ext_base_type Is the type of the pointer that is the internal base. Generall const 'void *' or 'void *'
 * @param ext_tensor_in_type Is the type of the external tensor used for import
 * @param ext_tensor_out_type Is the type of the external tensor used for export
 * @param int_bases The number of int base address required.
 *
 * Contains all the common elements of the pipeline schemes. The more
 * information that can be made common the more opportunity exists for future
 * optimizations and development.
 */
#define TTL_common_buffering_t(ext_base_type, ext_tensor_in_type, ext_tensor_out_type, int_bases)                 \
    struct {                                                                                                      \
        int index; /** @brief Describes the current buffer index when pipelining. For single 0->1->0, for double  \
                      0->1->0->1... etc */                                                                        \
        TTL_local(                                                                                                \
            ext_base_type) int_base[int_bases]; /** @brief The internal base addresses of the pipelined tiles. */ \
                                                                                                                  \
        ext_tensor_in_type ext_tensor_in;   /** @brief  The external tensor being input*/                         \
        ext_tensor_out_type ext_tensor_out; /** @brief  The external tensor being output*/                        \
    }

#undef TTL_INT_SUB_TENSOR_TYPE
#undef TTL_IO_TENSOR_TYPE
#define TTL_INT_SUB_TENSOR_TYPE __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t)
#define TTL_IO_TENSOR_TYPE __TTL_tensor_name(TTL_io_, , , TTL_TENSOR_TYPE, , _t)

/**
 * @brief Describes a pair of internal Tensors after an operation.
 *
 * The most likely usage is that compute input comes from the imported_to
 * TTL_int_tensor_t and the compute output goes to the to_export_from
 * TTL_int_tensor_t.
 *
 */
typedef struct {
    TTL_INT_SUB_TENSOR_TYPE imported_to;     ///< The TTL_int_sub_tensor_t that was most recently imported
    TTL_INT_SUB_TENSOR_TYPE to_export_from;  ///< The TTL_int_sub_tensor_t that will be exported next
} TTL_IO_TENSOR_TYPE;

/**
 * @brief Create a TTL_io_tensors_t from a pair of tensors
 *
 * @param imported_to The TTL_intTTL_int_sub_tensor_t_tensor_t that was most recently imported
 * @param to_export_from The TTL_iTTL_int_sub_tensor_tnt_tensor_t that will be exported next
 *
 * @return A TTL_io_tensors_t structure
 */
static inline TTL_IO_TENSOR_TYPE __attribute__((overloadable))
TTL_create_io_tensors(TTL_INT_SUB_TENSOR_TYPE imported_to, TTL_INT_SUB_TENSOR_TYPE to_export_from) {
    TTL_IO_TENSOR_TYPE result;
    result.imported_to = imported_to;
    result.to_export_from = to_export_from;

    return result;
}

static inline int __attribute__((overloadable)) TTL_tensors_empty(TTL_IO_TENSOR_TYPE tensors) {
    return TTL_int_tensor_empty(tensors.imported_to.tensor);
}

#endif