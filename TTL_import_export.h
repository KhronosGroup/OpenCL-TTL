/*
 * TTL_import_export.h
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

#include "TTL_core.h"
#include TTL_IMPORT_EXPORT_INCLUDE_H

#define TTL_MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

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
static inline TTL_local(void *) TTL_local_memset(TTL_local(void *) const ptr, char value, size_t num) {
    TTL_local(char *) const dst = (TTL_local(char *))ptr;

    for (size_t byte = 0; byte < num; byte++)
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

static inline TTL_shape_t TTL_import_pre_fill(const TTL_int_sub_tensor_t internal_sub_tensor,
                                              const TTL_const_ext_tensor_t const_external_tensor,
                                              TTL_local(void *) *const dst_address,
                                              TTL_global(void *) *const src_address) {
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
                   //     1 /*internal_sub_tensor.origin.shape.depth*/,
                   // 0);

    *dst_address = (TTL_local(char *))internal_sub_tensor.tensor.base +
                   (x_offset * internal_sub_tensor.tensor.elem_size) +
                   (y_offset * internal_sub_tensor.tensor.layout.row_spacing * internal_sub_tensor.tensor.elem_size) +
                   (z_offset * internal_sub_tensor.tensor.layout.plane_spacing * internal_sub_tensor.tensor.elem_size);

    *src_address = (TTL_global(char *))const_external_tensor.base + (x_offset * const_external_tensor.elem_size) +
                   (y_offset * const_external_tensor.layout.row_spacing * const_external_tensor.elem_size) +
                   (z_offset * const_external_tensor.layout.plane_spacing * const_external_tensor.elem_size);

    TTL_clear_void_space(internal_sub_tensor.tensor.base,
                         x_offset,
                         y_offset,
                         internal_sub_tensor.tensor.elem_size,
                         internal_sub_tensor.tensor.shape.width - x_cut,
                         internal_sub_tensor.tensor.layout.row_spacing,
                         internal_sub_tensor.tensor.shape.height - y_cut,
                         internal_sub_tensor.tensor.shape.height,
                         internal_sub_tensor.tensor.shape.depth);

    return TTL_create_shape(internal_sub_tensor.tensor.shape.width - x_offset - x_cut,
                            internal_sub_tensor.tensor.shape.height - y_offset - y_cut,
                            internal_sub_tensor.tensor.shape.depth - z_offset - z_cut);
}


#define TYPES_INCLUDE_FILE "import_export/TTL_typed_import_export.h"
#include "TTL_create_types.h"
