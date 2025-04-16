/*
 * TTL_simplex_scheme.h
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

// TTL_simplex_buffering_t
template <typename TENSORTYPE, typename TENSORSHAPETYPE, typename TILESHAPETYPE, typename LAYOUTTYPEIN,
          typename LAYOUTTYPEOUT>
struct TTL_simplex_buffering {
    /**
     * @brief Create a TTL_simplex_buffering and begin the buffering process
     *
     * @param int_base1 The address of the first buffer to be used in local memory
     * @param int_base2 The address of the second buffer to be used in local memory
     * @param int_base3 The address of the third buffer to be used in local memory
     * @param ext_tensor_in The external tensor to import the input data from
     * @param ext_tensor_out The external tensor to export the output data to
     * @param m_event_in A pointer to the event to use for the inward (external to
     * internal) transfer completion
     * @param m_event_out A pointer to the event to use for the inward (internal to
     * external) transfer completion
     * @param first_tile The first tile to fetch for the scheme
     *
     * Solid description of TTL_simplex_buffering buffering here
     *
     * @return The TTL_simplex_buffering created from the input parameters
     *
     * Example:
     * @code
     * TTL_event tb_e_in = TTL_get_event();
     * TTL_event tb_e_out = TTL_get_event();
     * TTL_simplex_buffering tb_scheme(
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
    TTL_simplex_buffering(TENSORTYPE *const int_base1, TENSORTYPE *const int_base2, TENSORTYPE *const int_base3,
                          const TTL_tensor<TENSORTYPE, TENSORSHAPETYPE, LAYOUTTYPEIN> &ext_tensor_in,
                          const TTL_tensor<TENSORTYPE, TENSORSHAPETYPE, LAYOUTTYPEOUT> &ext_tensor_out,
                          TTL_event *input_event_in, TTL_event *input_event_out,
                          const TTL_tile<TILESHAPETYPE> first_tile) {
        m_common.int_base[0] = int_base1;
        m_common.int_base[1] = int_base2;
        m_common.int_base[2] = int_base3;
        m_common.ext_tensor_in = ext_tensor_in;
        m_common.ext_tensor_out = ext_tensor_out;
        m_event_in = input_event_in;
        m_event_out = input_event_out;
        m_next_exported_tile = TTL_tile<TILESHAPETYPE>();

        m_common.index = 0;

        m_int_prev_imported = TTL_sub_tensor<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN>();

        step_buffering(first_tile, TTL_tile<TILESHAPETYPE>());
    }

    TTL_io_tensors<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN> 
	step_buffering(const TTL_tile<TILESHAPETYPE> &tile_next_import, const TTL_tile<TILESHAPETYPE> &tile_current_export) {
        // For performance, compute everything possible before waiting for the previous operations to finish. The
        // current index contains the tile that is to be exported, so prepare the structures before beginning the export
        // and export.
        const LAYOUTTYPEIN next_import_layout(tile_next_import.shape.width, tile_next_import.shape.height);
        const TTL_sub_tensor<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN> next_import_int_sub_tensor(
            m_common.int_base[m_common.index],
            tile_next_import.shape,
            next_import_layout,
            m_common.ext_tensor_in,
            tile_next_import.offset);
        const TTL_tensor<TENSORTYPE, TILESHAPETYPE, LAYOUTTYPEIN> next_import_ext_tensor(
            m_common.ext_tensor_in.base,
            tile_next_import.shape,
            m_common.ext_tensor_in.layout,
            tile_next_import.offset,
            m_common.ext_tensor_in.elem_size);

        const LAYOUTTYPEIN int_export_layout(m_next_exported_tile.shape.width, m_next_exported_tile.shape.height);
        const TTL_tensor<TENSORTYPE, TILESHAPETYPE, LAYOUTTYPEIN> int_export_tensor(m_common.int_base[m_common.index],
                                                                                    m_next_exported_tile.shape,
                                                                                    int_export_layout,
                                                                                    m_common.ext_tensor_out.elem_size);
        const TTL_tensor<TENSORTYPE, TILESHAPETYPE, LAYOUTTYPEOUT> export_to(m_common.ext_tensor_out.base,
                                                                             m_next_exported_tile.shape,
                                                                             m_common.ext_tensor_out.layout,
                                                                             m_next_exported_tile.offset,
                                                                             m_common.ext_tensor_out.elem_size);

        // Wait for the previous (import/export)s to complete before starting the next.
        TTL_wait(1, m_event_out);
        TTL_wait(1, m_event_in);

        if (m_next_exported_tile.empty() == false) TTL_export(int_export_tensor, export_to, m_event_out);

        // template <typename TENSORTYPE, typename SUBTENSORSHAPETYPE, typename ORIGINALTENSORSHAPETYPE>
        // void TTL_import_sub_tensor(
        //     const TTL_sub_tensor<TENSORTYPE, SUBTENSORSHAPETYPE, ORIGINALTENSORSHAPETYPE> &internal_sub_tensor,
        //     const TTL_tensor<TENSORTYPE, ORIGINALTENSORSHAPETYPE> const_external_tensor, TTL_event *event) {
        //     TTL_local(TENSORTYPE *) dst_address;
        //     TTL_global(TENSORTYPE *) src_address;

        if (tile_next_import.empty() == false)
            TTL_import_sub_tensor<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE>(
                next_import_int_sub_tensor, next_import_ext_tensor, m_event_in);

        // The import/export has been started for the current tile, Move to the next
        // tile.
        m_common.index = (m_common.index + 1) % TTL_ARRAYSIZE(m_common.int_base);  // Write to.

        // Retrieve buffer imported previously to read from now.
        const TTL_sub_tensor<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN> int_curr_buff_in =
            m_int_prev_imported;
        m_int_prev_imported = next_import_int_sub_tensor;

        // Can write to out buffer according to size of curr_tile, rather than size
        // recently exported.
        const LAYOUTTYPEIN curr_int_layout(tile_current_export.shape.width, tile_current_export.shape.width);
        const TTL_sub_tensor<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN> int_curr_buff_out(
            m_common.int_base[m_common.index],
            tile_current_export.shape,
            curr_int_layout,
            m_common.ext_tensor_in,
            tile_current_export.offset);

        // Save last two tiles to prevent m_common repeated get_tile()'s.
        m_next_exported_tile = tile_current_export;

        return TTL_io_tensors<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN>(int_curr_buff_in,
                                                                                        int_curr_buff_out);
    }

    /**
     * @brief Complete any transfers required to finish the buffering process.
     *
     * Any transfers that are still in progress will be completed and any transfers
     * that need to be started and completed before finish_buffering returns
     */
    void finish_buffering() {
        step_buffering(TTL_tile<TILESHAPETYPE>(), TTL_tile<TILESHAPETYPE>());
        step_buffering(TTL_tile<TILESHAPETYPE>(), TTL_tile<TILESHAPETYPE>());
    }

    TTL_common_buffering<TENSORTYPE, TENSORSHAPETYPE, LAYOUTTYPEIN, LAYOUTTYPEOUT, 3>
        m_common;  ///< The information that is common to all pipeline schemes

    TTL_event *m_event_in;
    TTL_event *m_event_out;
    // Cache previous gotten tiles.
    TTL_tile<TILESHAPETYPE> m_next_exported_tile;
    TTL_sub_tensor<TENSORTYPE, TILESHAPETYPE, TENSORSHAPETYPE, LAYOUTTYPEIN>
        m_int_prev_imported;  // Cache previously imported internal buffer.
};
