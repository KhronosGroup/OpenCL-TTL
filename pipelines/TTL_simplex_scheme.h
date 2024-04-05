/*
 * TTL_simplex_scheme.h
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

// clang-format off
/**
 * @file
 *
 * TTL_simplex_buffering pipelines a pair of import and export transactions using
 * three internal buffers, in rotation: each buffer interchangeably serves as input
 * buffer and output buffer, such that in each iteration one buffer is used both to
 * export then import and two buffers are used by compute for reading and writing.
 *
 * With simplex buffering we're only waiting for previous iterations, so DMA
 * transactions run mostly in parallel to computation, but serially with each
 * other. Using the same buffer both for import and export is possible allowing us
 * to overlap exporting from and importing to the same buffer.
 *
 * The following table draws the pipelined actions performed in simplex buffering.
 * It specifies which tile is processed in each iteration:
 *
 * | Action\\Iteration | \#-1 | \#0 | \#1 | \#2 | \#i (2:NumOfTiles-2) | \#NumOfTiles-1 | \#NumOfTiles | \#NumOfTiles+1 |
 * |-------------------|------|-----|-----|-----|----------------------|----------------|--------------|----------------|
 * | **WaitExport**    |      |     |     | 0   | i-2                  | NumOfTiles-3   | NumOfTiles-2 | NumOfTiles-1   |
 * | **Export**        |      |     | 0   | 1   | i-1                  | NumOfTiles-2   | NumOfTiles-1 |                |
 * | **Wait Import**   |      | 0   | 1   | 2   | i                    | NumOfTiles-1   |              |                |
 * | **Import**        | 0    | 1   | 2   | 3   | i+1                  |                |              |                |
 * | **Compute**       |      | 0   | 1   | 2   | i                    | NumOfTiles-1   |              |                |
 *
 * Notice the prolog (at iteration number -1) and the 2 epilogs (at iterations
 * number NumOfTiles and NumOfTiles+1) which add in total 3 extra iterations.
 *
 * @example TTL_simplex_buffering.cl
 */
// clang-format on

// This file presumes that the following have been pre included.
// this is not done here for path reasons.
// #include "TTL_core.h"
// #include "TTL_import_export.h"
// #include TTL_IMPORT_EXPORT_INCLUDE_H
#include "../TTL_macros.h"
#include "TTL_schemes_common.h"

/**
 * @def The structs used for this buffering type
 */
#undef TTL_IO_TENSOR_TYPE
#define TTL_IO_TENSOR_TYPE __TTL_tensor_name(TTL_io_, , , TTL_TENSOR_TYPE, , _t)
#undef TTL_SIMPLEX_BUFFERING_TYPE
#define TTL_SIMPLEX_BUFFERING_TYPE __TTL_tensor_name(TTL_simplex_, const_, , TTL_TENSOR_TYPE, , _buffering_t)
#undef TTL_INT_SUB_TENSOR_TYPE
#define TTL_INT_SUB_TENSOR_TYPE __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t)
#undef TTL_INT_TENSOR_TYPE
#define TTL_INT_TENSOR_TYPE __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, , _t)
#undef TTL_EXT_TENSOR_TYPE
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TTL_TENSOR_TYPE, , _t)
#undef TTL_CONST_EXT_TENSOR_TYPE
#define TTL_CONST_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, const_, ext_, TTL_TENSOR_TYPE, , _t)

// TTL_simplex_buffering_t
typedef struct {
    TTL_common_buffering_t(TTL_TENSOR_TYPE *, TTL_EXT_TENSOR_TYPE, TTL_EXT_TENSOR_TYPE,
                           3) common;  ///< The information that is common to all pipeline schemes

    TTL_event_t *event_in;
    TTL_event_t *event_out;
    // Cache previous gotten tiles.
    TTL_tile_t next_exported_tile;
    TTL_INT_SUB_TENSOR_TYPE int_prev_imported;  // Cache previously imported internal buffer.
} TTL_SIMPLEX_BUFFERING_TYPE;

/**
 * Simple declarations for file ordering purposes
 */
static inline TTL_IO_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_step_buffering, TTL_SIMPLEX_BUFFERING_TYPE *const simplex_buffer, TTL_tile_t tile_next_import,
               TTL_tile_t tile_current_export);

/**
 * @brief Create a TTL_simplex_buffering_t and begin the buffering process
 *
 * @param int_base1 The address of the first buffer to be used in local memory
 * @param int_base2 The address of the second buffer to be used in local memory
 * @param int_base3 The address of the third buffer to be used in local memory
 * @param ext_tensor_in The external tensor to import the input data from
 * @param ext_tensor_out The external tensor to export the output data to
 * @param event_in A pointer to the event to use for the inward (external to
 * internal) transfer completion
 * @param event_out A pointer to the event to use for the inward (internal to
 * external) transfer completion
 * @param first_tile The first tile to fetch for the scheme
 *
 * Solid description of TTL_double_double_buffering_t buffering here
 *
 * @return The TTL_simplex_buffering_t created from the input parameters
 *
 * Example:
 * @code
 * TTL_event_t tb_e_in = TTL_get_event();
 * TTL_event_t tb_e_out = TTL_get_event();
 * TTL_simplex_buffering_t tb_scheme = TTL_start_simplex_buffering(
 *       ext_base_in, ext_base_out, l_buff1, l_buff2, l_buff3, ext_layout_in,
 *       ext_layout_out, &tb_e_in, &tb_e_out);
 * @endcode
 * \n
 *
 * This can be optimized and standardized using the TTL_step_buffering
 * call.
 *
 * @startuml
 *
 * start
 *
 *
 * stop
 *
 * @enduml
 *
 */
static inline TTL_SIMPLEX_BUFFERING_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_start_simplex_buffering, TTL_local(TTL_TENSOR_TYPE *) int_base1,
               TTL_local(TTL_TENSOR_TYPE *) int_base2, TTL_local(TTL_TENSOR_TYPE *) int_base3,
               TTL_EXT_TENSOR_TYPE ext_tensor_in, TTL_EXT_TENSOR_TYPE ext_tensor_out, TTL_event_t *event_in,
               TTL_event_t *event_out, TTL_tile_t first_tile) {
    TTL_SIMPLEX_BUFFERING_TYPE result;

    result.common.int_base[0] = int_base1;
    result.common.int_base[1] = int_base2;
    result.common.int_base[2] = int_base3;
    result.common.ext_tensor_in = ext_tensor_in;
    result.common.ext_tensor_out = ext_tensor_out;
    result.event_in = event_in;
    result.event_out = event_out;
    result.next_exported_tile = TTL_create_empty_tile();

    result.common.index = 0;

    result.int_prev_imported = TTL_create_empty_int_sub_tensor(int_base1);

    TTL_step_buffering(&result, first_tile, TTL_create_empty_tile() __TTL_TRACE_LINE);

    return result;
}

static inline TTL_IO_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_step_buffering, TTL_SIMPLEX_BUFFERING_TYPE *const simplex_buffer, TTL_tile_t tile_next_import,
               TTL_tile_t tile_current_export) {
    // For performance, compute everything possible before waiting for the previous operations to finish. The current
    // index contains the tile that is to be exported, so prepare the structures before beginning the export and export.
    const TTL_layout_t next_import_layout =
        TTL_create_layout(tile_next_import.shape.width, tile_next_import.shape.height);
    const TTL_INT_SUB_TENSOR_TYPE next_import_int_sub_tensor =
        TTL_create_int_sub_tensor(simplex_buffer->common.int_base[simplex_buffer->common.index],
                                  tile_next_import.shape,
                                  next_import_layout,
                                  *TTL_to_const_tensor(&simplex_buffer->common.ext_tensor_in),
                                  tile_next_import.offset);
    const TTL_CONST_EXT_TENSOR_TYPE next_import_ext_tensor =
        TTL_create_const_ext_tensor(simplex_buffer->common.ext_tensor_in.base,
                                    tile_next_import.shape,
                                    simplex_buffer->common.ext_tensor_in.layout,
                                    tile_next_import.offset,
                                    simplex_buffer->common.ext_tensor_in.elem_size);

    const TTL_layout_t int_export_layout = TTL_create_layout(simplex_buffer->next_exported_tile.shape.width,
                                                             simplex_buffer->next_exported_tile.shape.height);
    const TTL_INT_TENSOR_TYPE int_export_tensor =
        TTL_create_int_tensor(simplex_buffer->common.int_base[simplex_buffer->common.index],
                              simplex_buffer->next_exported_tile.shape,
                              int_export_layout,
                              simplex_buffer->common.ext_tensor_out.elem_size);
    const TTL_EXT_TENSOR_TYPE export_to = TTL_create_ext_tensor(simplex_buffer->common.ext_tensor_out.base,
                                                                simplex_buffer->next_exported_tile.shape,
                                                                simplex_buffer->common.ext_tensor_out.layout,
                                                                simplex_buffer->next_exported_tile.offset,
                                                                simplex_buffer->common.ext_tensor_out.elem_size);

    // Wait for the previous (import/export)s to complete before starting the next.
    TTL_wait(1, simplex_buffer->event_out __TTL_TRACE_LINE);
    TTL_wait(1, simplex_buffer->event_in __TTL_TRACE_LINE);

    if (TTL_tile_empty(simplex_buffer->next_exported_tile) == false)
        TTL_export(*TTL_to_const_tensor(TTL_to_void_tensor(&int_export_tensor)),
                   *TTL_to_void_tensor(&export_to),
                   simplex_buffer->event_out __TTL_TRACE_LINE);

    if (TTL_tile_empty(tile_next_import) == false)
        TTL_import_sub_tensor(*TTL_to_void_sub_tensor(&next_import_int_sub_tensor),
                              *TTL_to_void_tensor(TTL_to_const_tensor(&next_import_ext_tensor)),
                              simplex_buffer->event_in __TTL_TRACE_LINE);

    // The import/export has been started for the current tile, Move to the next
    // tile.
    simplex_buffer->common.index =
        (simplex_buffer->common.index + 1) % TTL_ARRAYSIZE(simplex_buffer->common.int_base);  // Write to.

    // Retrieve buffer imported previously to read from now.
    const TTL_INT_SUB_TENSOR_TYPE int_curr_buff_in = simplex_buffer->int_prev_imported;
    simplex_buffer->int_prev_imported = next_import_int_sub_tensor;

    // Can write to out buffer according to size of curr_tile, rather than size
    // recently exported.
    const TTL_layout_t curr_int_layout =
        TTL_create_layout(tile_current_export.shape.width, tile_current_export.shape.width);
    const TTL_INT_SUB_TENSOR_TYPE int_curr_buff_out =
        TTL_create_int_sub_tensor(simplex_buffer->common.int_base[simplex_buffer->common.index],
                                  tile_current_export.shape,
                                  curr_int_layout,
                                  *TTL_to_const_tensor(&simplex_buffer->common.ext_tensor_in),
                                  tile_current_export.offset);

    // Save last two tiles to prevent common repeated get_tile()'s.
    simplex_buffer->next_exported_tile = tile_current_export;

    return TTL_create_io_tensors(int_curr_buff_in, int_curr_buff_out);
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_finish_buffering, TTL_SIMPLEX_BUFFERING_TYPE *const simplex_buffering) {
    TTL_step_buffering(simplex_buffering, TTL_create_empty_tile(), TTL_create_empty_tile() __TTL_TRACE_LINE);
    TTL_step_buffering(simplex_buffering, TTL_create_empty_tile(), TTL_create_empty_tile() __TTL_TRACE_LINE);
}
