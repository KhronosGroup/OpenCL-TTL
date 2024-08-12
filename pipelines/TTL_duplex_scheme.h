/*
 * TTL_duplex_scheme.h
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
 * Given pair of blocking import and export that can execute concurrently,
 * TTL_duplex_buffering issues them together and then waits on both to complete,
 * hopefully executing them in parallel to each other. This scheme uses two
 * internal buffers, one for the import and one for the export. Note that the
 * export is pipelined to pair the import of the current tile with the export of
 * previous tile.

 * The following table draws the pipelined actions performed in duplex buffering.
 * It specifies which tile is processed in each iteration:
 *
 * | Action\\Iteration | \#0 | \#1 | \#i (2:NumOfTiles-1) | \#NumOfTiles- |
 * |-------------------|-----|-----|----------------------|---------------|
 * | **Import**        | 0   | 1   | i                    |               |
 * | **Wait Import**   | 0   | 1   | i                    |               |
 * | **Compute**       | 0   | 1   | i                    |               |
 * | **Export**        |     | 0   | i-1                  | NumOfTiles-1  |
 * | **WaitExport**    |     | 0   | i-1                  | NumOfTiles-1  |
 *
 * Notice the epilog (\#NumOfTiles) which is an extra iteration.
 *
 * When including this file the following must be defined
 *
 * #define TTL_TENSOR_TYPE void
 * #define TTL_TENSOR_TYPE uchar
 * etc
 *
 * @example TTL_duplex_buffering.cl
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
#define TTL_DUPLEX_BUFFERING_TYPE __TTL_tensor_name(TTL_duplex_, const_, , TTL_TENSOR_TYPE, , _buffering_t)
#define TTL_INT_SUB_TENSOR_TYPE __TTL_tensor_name(TTL_, , int_, TTL_TENSOR_TYPE, sub_, _t)
#define TTL_CONST_INT_TENSOR_TYPE __TTL_tensor_name(TTL_, const_, int_, TTL_TENSOR_TYPE, , _t)
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TTL_TENSOR_TYPE, , _t)
#define TTL_CONST_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, const_, ext_, TTL_TENSOR_TYPE, , _t)
#define TTL_IO_TENSOR_TYPE __TTL_tensor_name(TTL_io_, , , TTL_TENSOR_TYPE, , _t)

/**
 * @brief Data required to perform duplex buffer pipelining.
 *
 * @see TTL_start_duplex_buffering for a description of duplex buffer
 * pipelining.
 */
typedef struct {
    TTL_common_buffering_t(TTL_TENSOR_TYPE *, TTL_EXT_TENSOR_TYPE, TTL_EXT_TENSOR_TYPE,
                           2) common;  ///< The information that is common to all pipeline schemes

    TTL_event_t (*events)[2];  ///< 2 Events are required, 1 first is used for
                               ///< external to internal transfers, the second for
                               ///< internal to external transfers

    /**
     * @brief Store of the buffers used for the previous import/export cycles.
     *
     */
    struct {
        TTL_EXT_TENSOR_TYPE to_export_to;
        __TTL_tensor_name(TTL_, const_, int_, TTL_TENSOR_TYPE, , _t) to_export_from;
    } prev_out_tensors;
} TTL_DUPLEX_BUFFERING_TYPE;

/*
 * Predeclare TTL_step_buffering.
 */
static inline TTL_IO_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_step_buffering, TTL_DUPLEX_BUFFERING_TYPE *const duplex_buffering, TTL_tile_t tile_next_import,
               TTL_tile_t tile_current_export);

/**
 * @brief Create a TTL_DUPLEX_BUFFERING_TYPE and begin the buffering process
 *
 * @param ext_tensor_in A tensor describing the input in global memory
 * @param int_base_in The address of the local import buffer.
 * @param ext_tensor_out A tensor describing the output in global memory
 * @param int_base_out The address of the local export buffer.
 * @param events A pointer to a list of 2 events.
 * The first event in the list will be used for imports, the second event in
 * the list will be used for exports.
 * @param first_tile The first tile to fetch for the scheme
 *
 * @return The TTL_DUPLEX_BUFFERING_TYPE created from the input parameters.
 *
 * The first event in the list will be used for imports,
 * the second event in the list will be used for exports.
 * \n\n Example:
 * @code
 * TTL_event_t events[2] = { TTL_get_event(), TTL_get_event()};
 *
 * TTL_duplex_buffering_t buffering_scheme = TTL_start_duplex_buffering(
 *     ext_base_in, ext_layout_in, l_buffers[0],
 *     ext_base_out, ext_layout_out, l_buffers[1],
 *     &events);
 * @endcode
 * \n
 *
 * @return The TTL_duplex_buffering_t created from the input parameters.
 *
 * Solid description of duplex buffering here.
 *
 * The simplest form of duplex buffering takes the following flow.
 *
 * @startuml
 *
 * start
 *
 * :Create a TTL_tiler_t with TTL_create_tiler;
 * :Create a TTL_duplex_buffering_t Structure with 2 Buffers
 * 1 input buffer, 1 output buffer;
 * :NumberOfTiles = TTL_number_of_tiles(tiler);
 *
 * while (for each tile)
 *
 *   :Import The Next Tile into the input buffer;
 *
 *   :Process the Tile from the input buffer to the output buffer;
 *
 *   :ExportThe Process Tile from into the output buffer;
 *
 * endwhile
 *
 * stop
 *
 * @enduml
 *
 * This can be optimized and standardized using the TTL_step_buffering
 * call.
 *
 * @startuml
 *
 * start
 *
 * :Create a TTL_tiler_t with TTL_create_tiler;
 * :Create a TTL_duplex_buffering_t Structure with 2 Buffers
 * 1 input buffer, 1 output buffer;
 * :NumberOfTiles = TTL_number_of_tiles(tiler);
 *
 * for each tile:
 *
 *   :Call TTL_step_buffering for the current tile
 *
 *    This will import the current new tile and export the last tile
 *    in parallel;
 *
 *   if (Does the input buffer contain a valid tile? **TTL_tile_empty(...)**) then (yes)
 *      :Process the Tile from the input buffer to the output buffer;
 *   endif
 *
 * endwhile
 *
 * stop
 *
 * @enduml
 */
static inline TTL_DUPLEX_BUFFERING_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_start_duplex_buffering, TTL_EXT_TENSOR_TYPE ext_tensor_in, TTL_local(TTL_TENSOR_TYPE *) int_base_in,
               TTL_EXT_TENSOR_TYPE ext_tensor_out, TTL_local(TTL_TENSOR_TYPE *) int_base_out, TTL_event_t (*events)[2],
               TTL_tile_t first_tile) {
    TTL_DUPLEX_BUFFERING_TYPE result;
    result.common.int_base[0] = int_base_in;
    result.common.int_base[1] = int_base_out;

    result.common.ext_tensor_in = ext_tensor_in;
    result.common.ext_tensor_out = ext_tensor_out;
    result.events = events;
    result.prev_out_tensors.to_export_to = TTL_create_empty_ext_tensor((TTL_global(TTL_TENSOR_TYPE *))0);
    result.prev_out_tensors.to_export_from = TTL_create_empty_const_int_tensor((TTL_local(TTL_TENSOR_TYPE *))0);

    TTL_step_buffering(&result, first_tile, TTL_create_empty_tile() __TTL_TRACE_LINE);

    return result;
}

static inline TTL_IO_TENSOR_TYPE __attribute__((overloadable))
__TTL_TRACE_FN(TTL_step_buffering, TTL_DUPLEX_BUFFERING_TYPE *const duplex_buffering, TTL_tile_t tile_current_import,
               TTL_tile_t tile_current_export) {
    const TTL_layout_t next_import_layout =
        TTL_create_layout(tile_current_import.shape.width, tile_current_import.shape.height);
    const TTL_CONST_EXT_TENSOR_TYPE next_import_ext_tensor =
        TTL_create_const_ext_tensor(duplex_buffering->common.ext_tensor_in.base,
                                    tile_current_import.shape,
                                    duplex_buffering->common.ext_tensor_in.layout,
                                    tile_current_import.offset,
                                    duplex_buffering->common.ext_tensor_in.elem_size);
    const TTL_INT_SUB_TENSOR_TYPE next_import_int_sub_tensor =
        TTL_create_int_sub_tensor(duplex_buffering->common.int_base[0],
                                  tile_current_import.shape,
                                  next_import_layout,
                                  *TTL_to_const_tensor(&duplex_buffering->common.ext_tensor_in),
                                  tile_current_import.offset);

    const TTL_CONST_INT_TENSOR_TYPE  next_export_int_tensor =
        duplex_buffering->prev_out_tensors.to_export_from;
    const TTL_EXT_TENSOR_TYPE next_export_ext_tensor = duplex_buffering->prev_out_tensors.to_export_to;

    if (TTL_tile_empty(tile_current_import) == false)
        TTL_import_sub_tensor(*TTL_to_void_sub_tensor(&next_import_int_sub_tensor),
                              *TTL_to_void_tensor(&next_import_ext_tensor),
                              &(*duplex_buffering->events)[0] __TTL_TRACE_LINE);

    if (TTL_const_int_tensor_empty(duplex_buffering->prev_out_tensors.to_export_from) == false)
        TTL_export(*TTL_to_void_tensor(&next_export_int_tensor),
                   *TTL_to_void_tensor(&next_export_ext_tensor),
                   &(*duplex_buffering->events)[1] __TTL_TRACE_LINE);

    const TTL_layout_t int_export_layout =
        TTL_create_layout(tile_current_export.shape.width, tile_current_export.shape.height);
    const TTL_EXT_TENSOR_TYPE to_export_to = TTL_create_ext_tensor(duplex_buffering->common.ext_tensor_out.base,
                                                                   tile_current_export.shape,
                                                                   duplex_buffering->common.ext_tensor_out.layout,
                                                                   tile_current_export.offset,
                                                                   duplex_buffering->common.ext_tensor_out.elem_size);
    const TTL_INT_SUB_TENSOR_TYPE to_export_from =
        TTL_create_int_sub_tensor(duplex_buffering->common.int_base[1],
                                  tile_current_export.shape,
                                  int_export_layout,
                                  *TTL_to_const_tensor(&duplex_buffering->common.ext_tensor_in),
                                  tile_current_export.offset);

    duplex_buffering->prev_out_tensors.to_export_to = to_export_to;
    duplex_buffering->prev_out_tensors.to_export_from = *TTL_to_const_tensor(&to_export_from.tensor);

    TTL_wait(2, *duplex_buffering->events);

    return TTL_create_io_tensors(next_import_int_sub_tensor, to_export_from);
}

static inline void __attribute__((overloadable))
__TTL_TRACE_FN(TTL_finish_buffering, TTL_DUPLEX_BUFFERING_TYPE *const duplex_buffering) {
    TTL_step_buffering(duplex_buffering, TTL_create_empty_tile(), TTL_create_empty_tile() __TTL_TRACE_LINE);
}
