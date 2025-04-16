/*
 * TTL_duplex_scheme.h
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

#include "../TTL_macros.h"
#include "TTL_schemes_common.h"

/**
 * @brief Data required to perform duplex buffer pipelining.
 *
 * @see TTL_start_duplex_buffering for a description of duplex buffer
 * pipelining.
 */
template <typename TENSORTYPE>
struct TTL_duplex_buffering {
    /**
     * @brief Create a TTL_DUPLEX_BUFFERING_TYPE and begin the buffering process
     *
     * @param ext_tensor_in A tensor describing the input in global memory
     * @param int_base_in The address of the local import buffer.
     * @param ext_tensor_out A tensor describing the output in global memory
     * @param int_base_out The address of the local export buffer.
     * @param m_events A pointer to a list of 2 m_events.
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
     * TTL_event m_events[2] = { TTL_get_event(), TTL_get_event()};
     *
     * TTL_duplex_buffering buffering_scheme(
     *     ext_base_in, ext_layout_in, l_buffers[0],
     *     ext_base_out, ext_layout_out, l_buffers[1],
     *     &m_events);
     * @endcode
     * \n
     *
     * @return The TTL_duplex_buffering created from the input parameters.
     *
     * Solid description of duplex buffering here.
     *
     * The simplest form of duplex buffering takes the following flow.
     *
     * @startuml
     *
     * start
     *
     * :Create a TTL_tiler with TTL_create_tiler;
     * :Create a TTL_duplex_buffering Structure with 2 Buffers
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
     * This can be optimized and standardized using the step_buffering
     * call.
     *
     * @startuml
     *
     * start
     *
     * :Create a TTL_tiler with TTL_create_tiler;
     * :Create a TTL_duplex_buffering Structure with 2 Buffers 1 input buffer, 1 output buffer;
     * :NumberOfTiles = TTL_number_of_tiles(tiler);
     *
     * while (for each tile)
     *
     *   :Call step_buffering for the current tile
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
    TTL_duplex_buffering(TTL_tensor<TENSORTYPE> ext_tensor_in, TTL_local(TENSORTYPE *) int_base_in,
                         TTL_tensor<TENSORTYPE> ext_tensor_out, TTL_local(TENSORTYPE *) int_base_out,
                         TTL_event (*input_events)[2], TTL_tile first_tile) {
        m_common.int_base [IMPORT_BUFFER] = int_base_in;
        m_common.int_base [EXPORT_BUFFER] = int_base_out;

        m_common.ext_tensor_in = ext_tensor_in;
        m_common.ext_tensor_out = ext_tensor_out;
        m_events = input_events;
        m_prev_out_tensors.to_export_to = TTL_tensor<TENSORTYPE>();
        m_prev_out_tensors.to_export_from = TTL_tensor<TENSORTYPE>();

        step_buffering(first_tile, TTL_tile());
    }

    TTL_io_tensors<TENSORTYPE> step_buffering(TTL_tile tile_current_import, TTL_tile tile_current_export) {
        const TTL_layout next_import_layout(tile_current_import.shape.width, tile_current_import.shape.height);
        const TTL_tensor<TENSORTYPE> next_import_ext_tensor(m_common.ext_tensor_in.base,
                                                            tile_current_import.shape,
                                                            m_common.ext_tensor_in.layout,
                                                            tile_current_import.offset,
                                                            m_common.ext_tensor_in.elem_size);
        const TTL_sub_tensor<TENSORTYPE> next_import_int_sub_tensor(m_common.int_base [IMPORT_BUFFER],
                                                                tile_current_import.shape,
                                                                next_import_layout,
                                                                m_common.ext_tensor_in,
                                                                tile_current_import.offset);

        const TTL_tensor<TENSORTYPE> next_export_int_tensor = m_prev_out_tensors.to_export_from;
        const TTL_tensor<TENSORTYPE> next_export_ext_tensor = m_prev_out_tensors.to_export_to;

        if (tile_current_import.empty() == false)
            TTL_import_sub_tensor(next_import_int_sub_tensor,
                                  next_import_ext_tensor,
                                  &(*m_events) [IMPORT_BUFFER]);

        if (m_prev_out_tensors.to_export_from.empty() == false)
            TTL_export(next_export_int_tensor,
                       next_export_ext_tensor,
                       &(*m_events) [EXPORT_BUFFER]);

        const TTL_layout int_export_layout(tile_current_export.shape.width, tile_current_export.shape.height);
        const TTL_tensor<TENSORTYPE> to_export_to(m_common.ext_tensor_out.base,
                                                  tile_current_export.shape,
                                                  m_common.ext_tensor_out.layout,
                                                  tile_current_export.offset,
                                                  m_common.ext_tensor_out.elem_size);
        const TTL_sub_tensor<TENSORTYPE> to_export_from(m_common.int_base [EXPORT_BUFFER],
                                      tile_current_export.shape,
                                      int_export_layout,
                                      m_common.ext_tensor_in,
                                      tile_current_export.offset);

        m_prev_out_tensors.to_export_to = to_export_to;
        m_prev_out_tensors.to_export_from = to_export_from.tensor;

        TTL_wait(2, *m_events);

        return TTL_io_tensors(next_import_int_sub_tensor, to_export_from);
    }

    	/**
	 * @brief Complete any transfers required to finish the buffering process.
	 * 
	 * Any transfers that are still in progress will be completed and any transfers
	 * that need to be started and completed before finish_buffering returns
	*/
void finish_buffering() {
        step_buffering(TTL_tile(), TTL_tile());
    }

    TTL_common_buffering<TENSORTYPE, 2> m_common;  ///< The information that is m_common to all pipeline schemes

	/**
	 * @brief Indexes to use for importing and exporting of data.
	*/
	static constexpr unsigned int IMPORT_BUFFER = 0;
	static constexpr unsigned int EXPORT_BUFFER = 1;

    TTL_event (*  m_events)[2];  ///< 2 m_events are required, 1 first is used for
                               ///< external to internal transfers, the second for
                               ///< internal to external transfers

    /**
     * @brief Store of the buffers used for the previous import/export cycles.
     *
     */
    struct {
        TTL_tensor<TENSORTYPE> to_export_to;
        TTL_tensor<TENSORTYPE> to_export_from;
    } m_prev_out_tensors;
};
