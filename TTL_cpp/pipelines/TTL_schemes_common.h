/*
 * TTL_schemes_common.h
 *
 * Copyright (c) 2025 Mobileye
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
template <typename TENSORTYPE, unsigned int BASES_COUNT>
struct TTL_common_buffering {
    int index; /*!< Describes the current buffer index when pipelining. For single 0->1->0, for double
                  0->1->0->1... etc */
    TTL_local(TENSORTYPE *) int_base[BASES_COUNT]; /*!< The internal base addresses of the pipelined tiles. */

    TTL_tensor<TENSORTYPE> ext_tensor_in;  /*!< The external tensor being input */
    TTL_tensor<TENSORTYPE> ext_tensor_out; /*!< The external tensor being output */
};

/**
 * @brief Describes a pair of internal Tensors after an operation.
 *
 * The most likely usage is that compute input comes from the imported_to
 * TTL_sub_tensor and the compute output goes to the to_export_from
 * TTL_sub_tensor.
 *
 */
template <typename TENSORTYPE>
struct TTL_io_tensors {
    /**
     * @brief Create a TTL_io_tensors from a pair of tensors
     *
     * @param imported_to The TTL_sub_tensor that was most recently imported
     * @param to_export_from The TTL_sub_tensor that will be exported next
     *
     * @return A TTL_io_tensors structure
     */
    TTL_io_tensors(TTL_sub_tensor<TENSORTYPE> imported_to, TTL_sub_tensor<TENSORTYPE> to_export_from)
        : imported_to(imported_to), to_export_from(to_export_from) {}

    bool empty() const {
        return imported_to.tensor.empty();
    }

    TTL_sub_tensor<TENSORTYPE> imported_to;     ///< The TTL_sub_tensor that was most recently imported
    TTL_sub_tensor<TENSORTYPE> to_export_from;  ///< The TTL_sub_tensor that will be exported next
};