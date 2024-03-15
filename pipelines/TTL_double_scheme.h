/*
 * TTL_double_scheme.h
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
// #include "TTL_import_export.h"
// #include TTL_IMPORT_EXPORT_INCLUDE_H
#include "../TTL_macros.h"
#include "TTL_schemes_common.h"

#define TTL_IMPORT_DOUBLE
#include "TTL_double_scheme_template.h"

#define TTL_EXPORT_DOUBLE
#include "TTL_double_scheme_template.h"

#pragma GCC diagnostic ignored  "-Wincompatible-pointer-types"

/**
 * @brief Wait for the previous import operation to complete before beginning an
 * import of the next tile.
 *
 * @param db TTL_import_double_buffering_t describing the attributes of the
 * transfer
 * @param next_tile A description of the tile to begin importing.
 *
 */
static inline TTL_INT_SUB_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_step_buffering, TTL_IMPORT_DOUBLE_BUFFERING_TYPE *const db, const TTL_tile_t next_tile) {
    // For performance, compute everything possible before waiting for the
    // previous operations to finish.
    const TTL_layout_t int_layout = TTL_create_layout(next_tile.shape.width, next_tile.shape.height);

    TTL_wait(1, db->event);

    if (TTL_tile_empty(next_tile) == false) {
        // I'll try and push this down the stack - but it gives the idea.
        if (next_tile.row_gather_map == 0) {
            const TTL_INT_SUB_TENSOR_TYPE import_to = TTL_create_int_sub_tensor(db->common.int_base[db->common.index],
                                                                                next_tile.shape,
                                                                                int_layout,
                                                                                db->common.ext_tensor_in,
                                                                                next_tile.offset);

            const TTL_CONST_EXT_TENSOR_TYPE import_from =
                TTL_create_const_ext_tensor(db->common.ext_tensor_in.base,
                                            next_tile.shape,
                                            db->common.ext_tensor_in.layout,
                                            next_tile.offset,
                                            db->common.ext_tensor_in.elem_size);

            TTL_import_sub_tensor(import_to, import_from, db->event __TTL_TRACE_LINE);
        } else {
            TTL_dim_t height_remaining = next_tile.shape.height;
            TTL_offset_t local_offset = {0, 0, 0};

            for (TTL_offset_dim_t current_row_offset = next_tile.offset.y;
                 height_remaining >0 ;
                 /* Increment in loop */) {
                const TTL_dim_t tile_height =
                    (current_row_offset < 0)
                        ? -current_row_offset
                        : min(height_remaining, next_tile.row_gather_map->elements[current_row_offset].row_count);
                const TTL_offset_dim_t y_offset =
                    (current_row_offset < 0) ? current_row_offset
                                             : next_tile.row_gather_map->elements[current_row_offset].row_offset;

                const TTL_shape_t sub_shape = {
                    .width = next_tile.shape.width, .height = tile_height, .depth = next_tile.shape.depth
                };
                const TTL_offset_t int_sub_offset = {
                    .x = next_tile.offset.x, .y = y_offset, .z = next_tile.offset.z
                };

                const TTL_INT_SUB_TENSOR_TYPE import_to =
                    TTL_create_int_sub_tensor(db->common.int_base[db->common.index],
                                              sub_shape,
                                              int_layout,
                                              local_offset,
                                              db->common.ext_tensor_in,
                                              int_sub_offset);

                const TTL_offset_t sub_offset = { .x = next_tile.offset.x, .y = y_offset, .z = next_tile.offset.z };
                height_remaining = height_remaining - tile_height;
                current_row_offset = current_row_offset + tile_height;
                local_offset.y = local_offset.y + tile_height;

                const TTL_CONST_EXT_TENSOR_TYPE import_from =
                    TTL_create_const_ext_tensor(db->common.ext_tensor_in.base,
                                                sub_shape,
                                                db->common.ext_tensor_in.layout,
                                                sub_offset,
                                                db->common.ext_tensor_in.elem_size);

                TTL_import_sub_tensor(import_to, import_from, db->event __TTL_TRACE_LINE);
                TTL_wait(1, db->event);
            }
        }
    }

    db->common.index = (db->common.index + 1) % 2;  // TTL_ARRAYSIZE(db->common.int_base);

    const TTL_layout_t prev_int_layout = TTL_create_layout(db->prev_tile.shape.width, db->prev_tile.shape.height);
    const TTL_INT_SUB_TENSOR_TYPE result = TTL_create_int_sub_tensor(db->common.int_base[db->common.index],
                                                                     db->prev_tile.shape,
                                                                     prev_int_layout,
                                                                     db->common.ext_tensor_in,
                                                                     db->prev_tile.offset);

    db->prev_tile = next_tile;

    return result;
}

/**
 * @brief Wait for the previous import operation to complete before beginning an
 * export of next tile.
 *
 * @param db A TTL_export_double_buffering_t describing the attributes of the
 * transfer
 * @param tile_current A description of the tile to begin exporting.
 *
 */
static inline TTL_INT_SUB_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_step_buffering, TTL_EXPORT_DOUBLE_BUFFERING_TYPE *db, TTL_tile_t tile_current) {
    const TTL_layout_t int_layout = TTL_create_layout(db->prev_tile.shape.width, db->prev_tile.shape.height);
    const TTL_CONST_INT_TENSOR_TYPE export_from = TTL_create_const_int_tensor(
        db->common.int_base[db->common.index], db->prev_tile.shape, int_layout, db->common.ext_tensor_in.elem_size);
    const TTL_EXT_TENSOR_TYPE export_to = TTL_create_ext_tensor(db->common.ext_tensor_in.base,
                                                                db->prev_tile.shape,
                                                                db->common.ext_tensor_in.layout,
                                                                db->prev_tile.offset,
                                                                db->common.ext_tensor_in.elem_size);

    TTL_wait(1, db->event);

    if (TTL_tile_empty(db->prev_tile) == false)
        TTL_export(*TTL_to_void_tensor(TTL_to_const_tensor(&export_from)),
                   *TTL_to_void_tensor(&export_to),
                   db->event __TTL_TRACE_LINE);

    db->common.index = (db->common.index + 1) % 2;  // TTL_ARRAYSIZE(db->common.int_base);
    const TTL_layout_t curr_int_layout = TTL_create_layout(tile_current.shape.width, tile_current.shape.height);
    const TTL_INT_SUB_TENSOR_TYPE result = TTL_create_int_sub_tensor(db->common.int_base[db->common.index],
                                                                     tile_current.shape,
                                                                     curr_int_layout,
                                                                     *TTL_to_const_tensor(&db->common.ext_tensor_in),
                                                                     tile_current.offset);
    db->prev_tile = tile_current;

    return result;
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_finish_buffering, TTL_IMPORT_DOUBLE_BUFFERING_TYPE *import_double_buffering) {
    (void)import_double_buffering;
    // Nothing to do.
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_finish_buffering, TTL_EXPORT_DOUBLE_BUFFERING_TYPE *export_double_buffering) {
    TTL_step_buffering(export_double_buffering, TTL_create_empty_tile() __TTL_TRACE_LINE);
    TTL_step_buffering(export_double_buffering, TTL_create_empty_tile() __TTL_TRACE_LINE);
}
