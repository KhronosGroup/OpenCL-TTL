/*
 * TTL_import_export.h
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

#include "TTL_core.h"
#include TTL_IMPORT_EXPORT_INCLUDE_H

#define TTL_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/**
 * @brief  Import the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor describing the internal tensor.
 * @param external_tensor A TTL_int_tensor describing the const external tensor.
 * @param event A TTL_event type to allow detection of import completion.
 *
 * Complete description of what not how here.
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
void TTL_import(const INT_TENSORTYPE internal_tensor, const EXT_TENSORTYPE external_tensor, TTL_event *event) {
    return TTL_import_base(internal_tensor, external_tensor, event);
}

/**
 * @brief  Import the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor describing the internal tensor.
 * @param external_tensor A TTL_int_tensor describing the external tensor.
 *
 * Complete description of what not how here.
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
void TTL_blocking_import(const INT_TENSORTYPE &internal_tensor, const EXT_TENSORTYPE &external_tensor) {
    TTL_blocking_import_base(internal_tensor, external_tensor);
}

template <typename TENSORTYPE, typename SUBTENSORSHAPETYPE, typename ORIGINALTENSORSHAPETYPE,typename LAYOUTTYPE>
static inline SUBTENSORSHAPETYPE TTL_import_pre_fill(
    const TTL_sub_tensor<TENSORTYPE, SUBTENSORSHAPETYPE, ORIGINALTENSORSHAPETYPE,LAYOUTTYPE> internal_sub_tensor,
    const TTL_tensor<TENSORTYPE, SUBTENSORSHAPETYPE,LAYOUTTYPE> const_external_tensor,
    TTL_local(TENSORTYPE *) *const dst_address, TTL_global(TENSORTYPE *) *const src_address);

/**
 * @brief Implementation of TTL_import_sub_tensor
 *
 * @param internal_sub_tensor A TTL_int_tensor describing the internal tensor.
 * @param const_external_tensor A TTL_const_ext_tensor describing the external tensor.
 * @param event A TTL_event type to allow detection of import completion.
 *
 * @see TTL_import for full API and parameter information
 */
template <typename TENSORTYPE, typename SUBTENSORSHAPETYPE, typename ORIGINALTENSORSHAPETYPE,typename LAYOUTTYPE>
void TTL_import_sub_tensor(
    const TTL_sub_tensor<TENSORTYPE, SUBTENSORSHAPETYPE, ORIGINALTENSORSHAPETYPE,LAYOUTTYPE> &internal_sub_tensor,
    const TTL_tensor<TENSORTYPE, SUBTENSORSHAPETYPE,LAYOUTTYPE> const_external_tensor, TTL_event *event) {
    TTL_local(TENSORTYPE *) dst_address;
    TTL_global(TENSORTYPE *) src_address;

    const SUBTENSORSHAPETYPE import_shape = TTL_import_pre_fill<TENSORTYPE, SUBTENSORSHAPETYPE, ORIGINALTENSORSHAPETYPE>(
        internal_sub_tensor, const_external_tensor, &dst_address, &src_address);

    const TTL_tensor<TENSORTYPE, SUBTENSORSHAPETYPE,LAYOUTTYPE> import_int_tensor(
        dst_address, import_shape, internal_sub_tensor.tensor.layout, internal_sub_tensor.tensor.elem_size);

    const TTL_tensor<TENSORTYPE, SUBTENSORSHAPETYPE,LAYOUTTYPE> import_ext_tensor(
        src_address, import_shape, const_external_tensor.layout, TTL_offset(), const_external_tensor.elem_size);

    TTL_import(import_int_tensor, import_ext_tensor, event);
}

/**
 * @brief  Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor describing the internal tensor.
 * @param external_tensor A TTL_int_tensor describing the const external tensor.
 * @param event A TTL_event type to allow detection of import completion.
 *
 * Complete description of what not how here.
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
void TTL_export(const INT_TENSORTYPE &internal_tensor, const EXT_TENSORTYPE &external_tensor, TTL_event *event) {
    return TTL_export_base(internal_tensor, external_tensor, event);
}

/**
 * @brief  Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor describing the internal tensor.
 * @param external_tensor A TTL_int_tensor describing the const external tensor.
 *
 * Complete description of what not how here.
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
void TTL_blocking_export(const INT_TENSORTYPE &internal_tensor, const EXT_TENSORTYPE &external_tensor) {
    TTL_blocking_export_base(internal_tensor, external_tensor);
}

/**
 * @brief Fill block of local memory
 *
 * Sets the first num bytes of the block of memory pointed by ptr to the
 * specified value (interpreted as an unsigned char).
 *
 * @param ptr Pointer to the block of memory to fill.
 * @param value Value to be set. The value is passed as an int, but the function
 * fills the block of memory using the unsigned char conversion of this value.
 * @param num Number of bytes to be set to the value.
 *
 * @return ptr is to the output is returned.
 */
static inline TTL_local(void *) TTL_local_memset(TTL_local(void *) const ptr, char value, int num) {
    TTL_local(char *) const dst = (TTL_local(char *))ptr;

    for (int byte = 0; byte < num; byte++)
        dst[byte] = value;

    return ptr;
}

/**
 * @brief Clear any unpopulated space in the target area.
 *
 * @param dst
 * @param x_offset
 * @param y_offset
 * @param num_bytes_per_element
 * @param num_elements_per_line
 * @param dst_total_line_length
 * @param num_lines
 * @param total_lines
 * @param num_planes
 */
static inline void TTL_clear_void_space(TTL_local(void *) const dst, const size_t x_offset, const size_t y_offset,
                                        size_t num_bytes_per_element, size_t num_elements_per_line,
                                        size_t dst_total_line_length, size_t num_lines, size_t total_lines,
                                        size_t num_planes) {
    TTL_local(char *) dst_ptr = (TTL_local(char *))dst;
    unsigned int left_trim_bytes = x_offset * num_bytes_per_element;
    unsigned int right_trim_bytes = (dst_total_line_length - num_elements_per_line) * num_bytes_per_element;

    for (size_t plane = 0; plane < num_planes; plane++) {
        for (size_t line = 0; line < total_lines; line++) {
            if ((line < y_offset) || (line >= num_lines)) {
                TTL_local_memset(dst_ptr, 0, dst_total_line_length * num_bytes_per_element);
            } else {
                // Clear anything not being copied to zero - will make the 'clear value'
                // definable at some level.
                TTL_local_memset(dst_ptr, 0, left_trim_bytes);
                TTL_local_memset(dst_ptr + (num_elements_per_line * num_bytes_per_element), 0, right_trim_bytes);
            }
            dst_ptr += dst_total_line_length * num_bytes_per_element;
        }
    }
}

template <typename TENSORTYPE, typename INT_SHAPETYPE, typename EXT_SHAPETYPE,typename LAYOUTTYPE>
static inline INT_SHAPETYPE TTL_import_pre_fill(
    const TTL_sub_tensor<TENSORTYPE, INT_SHAPETYPE, EXT_SHAPETYPE,LAYOUTTYPE> internal_sub_tensor,
    const TTL_tensor<TENSORTYPE, INT_SHAPETYPE,LAYOUTTYPE> const_external_tensor,
    TTL_local(TENSORTYPE *) *const dst_address, TTL_global(TENSORTYPE *) *const src_address) {
    size_t x_offset;
    size_t x_cut;
    size_t y_offset;
    size_t y_cut;
    size_t z_offset;
    size_t z_cut;

    x_offset = TTL_MAX(-internal_sub_tensor.origin.sub_offset.x, 0);
    x_cut = (internal_sub_tensor.origin.sub_offset.x + internal_sub_tensor.tensor.shape.width) >
                    internal_sub_tensor.origin.shape.width
                ? (internal_sub_tensor.origin.sub_offset.x + internal_sub_tensor.tensor.shape.width) -
                      internal_sub_tensor.origin.shape.width
                : 0;

    y_offset = TTL_MAX(-internal_sub_tensor.origin.sub_offset.y, 0);
    y_cut = (internal_sub_tensor.origin.sub_offset.y + internal_sub_tensor.tensor.shape.height) >
                    internal_sub_tensor.origin.shape.height
                ? (internal_sub_tensor.origin.sub_offset.y + internal_sub_tensor.tensor.shape.height) -
                      internal_sub_tensor.origin.shape.height
                : 0;

    z_offset = 0;  // TTL_MAX(-internal_sub_tensor.origin.sub_offset.z, 0);
    z_cut = 0;     // TTL_MAX((internal_sub_tensor.origin.sub_offset.z + internal_sub_tensor.tensor.shape.depth) -
                   //     1 /* Internal_sub_tensor.origin.shape.depth */
                   // 0);

    *dst_address = internal_sub_tensor.tensor.base + x_offset +
                   (y_offset * internal_sub_tensor.tensor.layout.row_spacing) +
                   (z_offset * internal_sub_tensor.tensor.layout.plane_spacing);

    *src_address = const_external_tensor.base + x_offset + (y_offset * const_external_tensor.layout.row_spacing) +
                   (z_offset * const_external_tensor.layout.plane_spacing);

    TTL_clear_void_space(internal_sub_tensor.tensor.base,
                         x_offset,
                         y_offset,
                         internal_sub_tensor.tensor.elem_size,
                         internal_sub_tensor.tensor.shape.width - x_cut,
                         internal_sub_tensor.tensor.layout.row_spacing,
                         internal_sub_tensor.tensor.shape.height - y_cut,
                         internal_sub_tensor.tensor.shape.height,
                         internal_sub_tensor.tensor.shape.depth);

    return INT_SHAPETYPE(internal_sub_tensor.tensor.shape.width - x_offset - x_cut,
                         internal_sub_tensor.tensor.shape.height - y_offset - y_cut,
                         internal_sub_tensor.tensor.shape.depth - z_offset - z_cut);
}
