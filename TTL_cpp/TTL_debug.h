/*
 * TTL_debug.h
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

#include "TTL_tensors.h"
#include "TTL_tiles.h"
#include "TTL_types.h"

#if __TTL_DEBUG > 0

/**
 * @brief Print a debug copy of a TTL_shape type
 *
 * Output debug of the TTL_shape passed using printf
 *
 * @param ttl_shape The tile to print debug info for.
 */
static inline void __TTL_dump_shape_t(const TTL_shape *const ttl_shape) {
    printf("TTL_shape: %d,%d,%d ", ttl_shape->width, ttl_shape->height, ttl_shape->depth);
}

/**
 * @brief Print a debug copy of a TTL_layout type
 *
 * Output debug of the TTL_layout passed using printf
 *
 * @param ttl_layout The tile to print debug info for.
 */
static inline void __TTL_dump_layout_t(const TTL_layout *const ttl_layout) {
    printf("TTL_layout: %d,%d ", ttl_layout->row_spacing, ttl_layout->plane_spacing);
}

/**
 * @brief Print a debug copy of a TTL_offset type
 *
 * Output debug of the TTL_offset passed using printf
 *
 * @param ttl_offset The offset 3d to print debug info for.
 */
static inline void __TTL_dump_offset_t(const TTL_offset *const ttl_offset) {
    printf("TTL_offset: %d,%d,%d ", ttl_offset->x, ttl_offset->y, ttl_offset->z);
}

/**
 * @brief Print a debug copy of a TTL_overlap type
 *
 * Output debug of the TTL_overlap passed using printf
 *
 * @param ttl_overlap The overlap to print debug info for.
 */
static inline void __TTL_dump_overlap_t(const TTL_overlap *const ttl_overlap) {
    printf("TTL_overlap: %d,%d,%d ", ttl_overlap->width, ttl_overlap->height, ttl_overlap->depth);
}

/**
 * @brief Print a debug copy of a TTL_augmentation type
 *
 * Output debug of the TTL_augmentation passed using printf
 *
 * @param ttl_augmented The overlap to print debug info for.
 */
static inline void __TTL_dump_augmentation_t(const TTL_augmentation *const ttl_augmented) {
    printf("TTL_augmentation: (%d,%d),(%d,%d),(%d,%d), ",
           ttl_augmented->left,
           ttl_augmented->right,
           ttl_augmented->top,
           ttl_augmented->bottom,
           ttl_augmented->front,
           ttl_augmented->back);
}

/**
 * @brief Print a debug copy of a TTL_tile type
 *
 * Output debug of the TTL_tile passed using printf
 *
 * @param ttl_tile The tile to print debug info for.
 */
static inline void __TTL_dump_tile_t(const TTL_tile *const ttl_tile) {
    printf("TTL_tile: ");
    __TTL_dump_shape_t(&ttl_tile->shape);
    __TTL_dump_offset_t(&ttl_tile->offset);
}

/**
 * @brief Print a debug copy of a TTL_tensor type
 *
 * Output debug of the TTL_tile passed using printf
 *
 * @param ttl_int_tensor The internal tensor to print debug info for.
 */
template <typename TENSORTYPE>
static inline void __TTL_dump_tensor(const TTL_tensor<TENSORTYPE> *const ttl_int_tensor) {
    printf("TTL_int_tensor_t: %p,%d ", ttl_int_tensor->base, ttl_int_tensor->elem_size);
    __TTL_dump_layout_t(&ttl_int_tensor->layout);
    __TTL_dump_shape_t(&ttl_int_tensor->shape);
}

/**
 * @brief Print a debug copy of a TTL_sub_tensor type
 *
 * Output debug of the TTL_sub_tensor passed using printf
 *
 * @param ttl_int_sub_tensor The internal tensor to print debug info for.
 */
template <typename TENSORTYPE>
static inline void __TTL_dump_sub_tensor(const TTL_sub_tensor<TENSORTYPE> *const ttl_int_sub_tensor) {
    printf("TTL_int_sub_tensor_t: ");
    __TTL_dump_int_tensor_t(&ttl_int_sub_tensor->tensor);
    __TTL_dump_shape(&ttl_int_sub_tensor->origin.shape);
    __TTL_dump_offset_t(&ttl_int_sub_tensor->origin.sub_offset);
}

/**
 * @brief Print a debug copy of a TTL_tiler type
 *
 * Output debug of the TTL_tiler passed using printf
 *
 * @param ttl_tiler The tiler to print debug info for.
 */
static inline void __TTL_dump_tiler_t(const TTL_tiler *const ttl_tiler) {
    printf("TTL_tiler: ");
    __TTL_dump_shape_t(&ttl_tiler->space);
    __TTL_dump_shape_t(&ttl_tiler->tile);
    __TTL_dump_overlap_t(&ttl_tiler->overlap);
    printf("Cache: %d,%d,%d,%d,%d ",
           ttl_tiler->cache.number_of_tiles,
           ttl_tiler->cache.tiles_in_width,
           ttl_tiler->cache.tiles_in_height,
           ttl_tiler->cache.tiles_in_depth,
           ttl_tiler->cache.tiles_in_plane);
}

/**
 * @brief Internal non-API helper function to allow debugging of exports and
 * imports
 *
 * @param is_export
 * @param internal_tensor
 * @param external_tensor
 * @param access_type
 * @param event
 * @param line
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
static inline void __TTL_dump_transaction(const bool is_export, const TTL_tensor<INT_TENSORTYPE> *const internal_tensor,
                                          const TTL_tensor<EXT_TENSORTYPE> *const external_tensor,
                                          const int access_type, const TTL_event *const event,
                                          const unsigned int line) {
    printf(is_export ? "Export " : "Import ");
    __TTL_dump_shape_t(&internal_tensor->shape);
    __TTL_dump_event(event);
    printf(" AccessType: %d\n       ", access_type);
    __TTL_dump_const_ext_tensor_t(external_tensor);
    printf("\n       ");
    __TTL_dump_const_int_tensor_t(internal_tensor);
    printf("\n       line: %d\n", line);
}

static inline void __TTL_dump_wait(int num_events, TTL_event *events, const unsigned int line) {
    printf("TTL_WAIT: ");
    for (int i = 0; i < num_events; i++) {
        __TTL_dump_event(&events[i]);
    }
    printf("\n       line: %d\n", line);
}
#else  // NOT __TTL_DEBUG

static inline void __TTL_dump_wait(int /*num_events*/, TTL_event * /*events*/, const unsigned int /*line*/) {}

template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
static inline void __TTL_dump_transaction(const bool /*is_export*/,
                                          const TTL_tensor<INT_TENSORTYPE> & /*internal_tensor*/,
                                          const TTL_tensor<EXT_TENSORTYPE> & /*external_tensor*/,
                                          const int /*access_type*/, const TTL_event *const /*event*/,
                                          const unsigned int /*line*/){};

#endif  // __TTL_DEBUG
