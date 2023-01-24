/*
 * TTL_debug.h
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

#include "TTL_tensors.h"
#include "TTL_tiles.h"
#include "TTL_types.h"

/**
 * @def __TTL_create_dumper
 * @brief Create a call the provides a CR as well
 *
 * When using dump functions from the code we generally want a CR.
 */
#define __TTL_create_dumper(type)                                                \
    static inline void __TTL_dump_##type(const TTL_##type##_t *const variable) { \
        __TTL_dump_##type##_t(variable);                                         \
        printf("\n");                                                            \
    }

/**
 * @brief Print a debug copy of a TTL_shape_t type
 *
 * Output debug of the TTL_shape_t passed using printf
 *
 * @param ttl_shape The tile to print debug info for.
 */
static inline void __TTL_dump_shape_t(const TTL_shape_t *const ttl_shape) {
    printf("TTL_shape_t: %d,%d,%d ", ttl_shape->width, ttl_shape->height, ttl_shape->depth);
}

__TTL_create_dumper(shape);

/**
 * @brief Print a debug copy of a TTL_layout_t type
 *
 * Output debug of the TTL_layout_t passed using printf
 *
 * @param ttl_layout The tile to print debug info for.
 */
static inline void __TTL_dump_layout_t(const TTL_layout_t *const ttl_layout) {
    printf("TTL_layout_t: %d,%d ", ttl_layout->row_spacing, ttl_layout->plane_spacing);
}

__TTL_create_dumper(layout);

/**
 * @brief Print a debug copy of a TTL_offset_t type
 *
 * Output debug of the TTL_offset_t passed using printf
 *
 * @param ttl_offset The offset 3d to print debug info for.
 */
static inline void __TTL_dump_offset_t(const TTL_offset_t *const ttl_offset) {
    printf("TTL_offset_t: %d,%d,%d ", ttl_offset->x, ttl_offset->y, ttl_offset->z);
}

__TTL_create_dumper(offset);

/**
 * @brief Print a debug copy of a TTL_overlap_t type
 *
 * Output debug of the TTL_overlap_t passed using printf
 *
 * @param ttl_overlap The overlap to print debug info for.
 */
static inline void __TTL_dump_overlap_t(const TTL_overlap_t *const ttl_overlap) {
    printf("TTL_overlap_t: %d,%d,%d ", ttl_overlap->width, ttl_overlap->height, ttl_overlap->depth);
}

__TTL_create_dumper(overlap);

/**
 * @brief Print a debug copy of a TTL_augmentation_t type
 *
 * Output debug of the TTL_augmentation_t passed using printf
 *
 * @param ttl_augmented The overlap to print debug info for.
 */
static inline void __TTL_dump_augmentation_t(const TTL_augmentation_t *const ttl_augmented) {
    printf("TTL_augmentation_t: (%d,%d),(%d,%d),(%d,%d), ",
           ttl_augmented->left,
           ttl_augmented->right,
           ttl_augmented->top,
           ttl_augmented->bottom,
           ttl_augmented->front,
           ttl_augmented->back);
}

__TTL_create_dumper(augmentation);

/**
 * @brief Print a debug copy of a TTL_tile_t type
 *
 * Output debug of the TTL_tile_t passed using printf
 *
 * @param ttl_tile The tile to print debug info for.
 */
static inline void __TTL_dump_tile_t(const TTL_tile_t *const ttl_tile) {
    printf("TTL_tile_t: ");
    __TTL_dump_shape_t(&ttl_tile->shape);
    __TTL_dump_offset_t(&ttl_tile->offset);
}

__TTL_create_dumper(tile);

/**
 * @brief Print a debug copy of a TTL_int_tensor_t type
 *
 * Output debug of the TTL_tile_t passed using printf
 *
 * @param ttl_int_tensor The internal tensor to print debug info for.
 */
static inline void __TTL_dump_const_int_tensor_t(const TTL_const_int_tensor_t *const ttl_int_tensor) {
    printf("TTL_int_tensor_t: %p,%d ", ttl_int_tensor->base, ttl_int_tensor->elem_size);
    __TTL_dump_layout_t(&ttl_int_tensor->layout);
    __TTL_dump_shape_t(&ttl_int_tensor->shape);
}

static void __TTL_dump_int_tensor_t(const TTL_int_tensor_t *const ttl_int_tensor) {
    __TTL_dump_const_int_tensor_t(TTL_to_const_tensor(ttl_int_tensor));
}

__TTL_create_dumper(int_tensor);

/**
 * @brief Print a debug copy of a TTL_int_sub_tensor_t type
 *
 * Output debug of the TTL_int_sub_tensor_t passed using printf
 *
 * @param ttl_int_sub_tensor The internal tensor to print debug info for.
 */
static inline void __TTL_dump_int_sub_tensor_t(const TTL_int_sub_tensor_t *const ttl_int_sub_tensor) {
    printf("TTL_int_sub_tensor_t: ");
    __TTL_dump_int_tensor_t(&ttl_int_sub_tensor->tensor);
    __TTL_dump_shape(&ttl_int_sub_tensor->origin.shape);
    __TTL_dump_offset_t(&ttl_int_sub_tensor->origin.sub_offset);
}

__TTL_create_dumper(int_sub_tensor);

/**
 * @brief Print a debug copy of a TTL_ext_tensor_t type
 *
 * Output debug of the TTL_tile_t passed using printf
 *
 * @param ttl_ext_tensor The external tensor to print debug info for.
 */
static void __TTL_dump_const_ext_tensor_t(const TTL_const_ext_tensor_t *const ttl_const_ext_tensor) {
    printf("TTL_ext_tensor_t: " TTL_global_printf ",%d ",
           (TTL_global(void *))ttl_const_ext_tensor->base,
           ttl_const_ext_tensor->elem_size);
    __TTL_dump_layout_t(&ttl_const_ext_tensor->layout);
    __TTL_dump_shape_t(&ttl_const_ext_tensor->shape);
}

static void __TTL_dump_ext_tensor_t(const TTL_ext_tensor_t *const ttl_ext_tensor) {
    __TTL_dump_const_ext_tensor_t(TTL_to_const_tensor(ttl_ext_tensor));
}

__TTL_create_dumper(ext_tensor);

/**
 * @brief Print a debug copy of a TTL_tiler_t type
 *
 * Output debug of the TTL_tiler_t passed using printf
 *
 * @param ttl_tiler The tiler to print debug info for.
 */
static void __TTL_dump_tiler_t(const TTL_tiler_t *const ttl_tiler) {
    printf("TTL_tiler_t: ");
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

__TTL_create_dumper(tiler);

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
static inline void __TTL_dump_transaction(const bool is_export, const TTL_const_int_tensor_t *const internal_tensor,
                                          const TTL_const_ext_tensor_t *const external_tensor, const int access_type,
                                          const TTL_event_t *const event, const unsigned int line) {
    printf(is_export ? "Export " : "Import ");
    __TTL_dump_shape_t(&internal_tensor->shape);
    __TTL_dump_event(event);
    printf("AccessType: %d\n       ", access_type);
    __TTL_dump_const_ext_tensor_t(external_tensor);
    printf("\n       ");
    __TTL_dump_const_int_tensor_t(internal_tensor);
    printf("\n       line: %d\n", line);
}

static inline void __TTL_dump_wait(int num_events, TTL_event_t *events) {
    printf("TTL_WAIT: ");
    for (int i = 0; i < num_events; i++) {
        __TTL_dump_event(&events[i]);
    }
}
