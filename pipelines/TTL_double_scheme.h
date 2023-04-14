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

#pragma once

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

/**
 * @brief Wait for the previous import operation to complete before beginning an
 * import of the next tile.
 *
 * @param db TTL_import_double_buffering_t describing the attributes of the
 * transfer
 * @param next_tile A description of the tile to begin importing.
 *
 */
static inline TTL_int_sub_tensor_t __attribute__((overloadable)) __TTL_TRACE_FN(TTL_step_buffering, TTL_import_double_buffering_t *const db,
                                                  const TTL_tile_t next_tile) {
    // For performance, compute everything possible before waiting for the
    // previous operations to finish.
    const TTL_layout_t int_layout = TTL_create_layout(next_tile.shape.width, next_tile.shape.height);
    const TTL_int_sub_tensor_t import_to = TTL_create_int_sub_tensor(
        db->common.int_base[db->common.index], next_tile.shape, int_layout, db->common.ext_tensor_in, next_tile.offset);
    const TTL_const_ext_tensor_t import_from = TTL_create_const_ext_tensor(db->common.ext_tensor_in.base,
                                                                           next_tile.shape,
                                                                           db->common.ext_tensor_in.layout,
                                                                           next_tile.offset,
                                                                           db->common.ext_tensor_in.elem_size);

    TTL_wait(1, db->event);

    if (TTL_tile_empty(next_tile) == false) TTL_import_sub_tensor(import_to, import_from, db->event __TTL_TRACE_LINE);

    db->common.index = (db->common.index + 1) % 2;  // TTL_ARRAYSIZE(db->common.int_base);

    const TTL_layout_t prev_int_layout = TTL_create_layout(db->prev_tile.shape.width, db->prev_tile.shape.height);
    const TTL_int_sub_tensor_t result = TTL_create_int_sub_tensor(db->common.int_base[db->common.index],
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
static inline TTL_int_sub_tensor_t __TTL_TRACE_FN(TTL_step_buffering, TTL_export_double_buffering_t *db,
                                                  TTL_tile_t tile_current) {
    const TTL_layout_t int_layout = TTL_create_layout(db->prev_tile.shape.width, db->prev_tile.shape.height);
    const TTL_const_int_tensor_t export_from = TTL_create_const_int_tensor(
        db->common.int_base[db->common.index], db->prev_tile.shape, int_layout, db->common.ext_tensor_in.elem_size);
    const TTL_ext_tensor_t export_to = TTL_create_ext_tensor(db->common.ext_tensor_in.base,
                                                             db->prev_tile.shape,
                                                             db->common.ext_tensor_in.layout,
                                                             db->prev_tile.offset,
                                                             db->common.ext_tensor_in.elem_size);

    TTL_wait(1, db->event);

    if (TTL_tile_empty(db->prev_tile) == false) TTL_export(export_from, export_to, db->event __TTL_TRACE_LINE);

    db->common.index = (db->common.index + 1) % 2;  // TTL_ARRAYSIZE(db->common.int_base);
    const TTL_layout_t curr_int_layout = TTL_create_layout(tile_current.shape.width, tile_current.shape.height);
    const TTL_int_sub_tensor_t result =
        TTL_create_int_sub_tensor(db->common.int_base[db->common.index],
                                  tile_current.shape,
                                  curr_int_layout,
                                  *TTL_to_const_tensor(&db->common.ext_tensor_in),
                                  tile_current.offset);
    db->prev_tile = tile_current;

    return result;
}

static inline void __attribute__((overloadable)) __TTL_TRACE_FN(TTL_finish_buffering,
                                  TTL_import_double_buffering_t *import_double_buffering) {
    (void)import_double_buffering;
    // Nothing to do.
}

static inline void __attribute__((overloadable)) __TTL_TRACE_FN(TTL_finish_buffering,
                                  TTL_export_double_buffering_t *export_double_buffering) {
    TTL_step_buffering(export_double_buffering, TTL_create_empty_tile() __TTL_TRACE_LINE);
    TTL_step_buffering(export_double_buffering, TTL_create_empty_tile() __TTL_TRACE_LINE);
}
