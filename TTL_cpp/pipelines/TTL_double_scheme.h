/*
 * TTL_double_scheme.h
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

// This file presumes that the following have been pre included.
// this is not done here for path reasons.
// #include "TTL_core.h"
// #include "TTL_import_export.h"
// #include TTL_IMPORT_EXPORT_INCLUDE_H
#include "../TTL_macros.h"
#include "TTL_schemes_common.h"

/**
 * @brief Wait for the previous import operation to complete before beginning an
 * import of the next tile.
 *
 * @param db TTL_import_double_buffering describing the attributes of the
 * transfer
 * @param next_tile A description of the tile to begin importing.
 *
 */
template <typename TENSORTYPE>
struct TTL_import_double_buffering {
    /**
     * @brief Create a TTL_import_double_buffering and begin the buffering process
     *
     * @param int_base1 A pointer to the 1st local buffer
     * @param int_base2 A pointer to the 2nd local buffer
     * @param ext_tensor A tensor describing the input in global memory
     * @param event A pointer to the event to use for the inward (external to
     * internal) transfer completion
     * @param first_tile The first tile to fetch for the scheme
     *
     * @return The TTL_import_double_buffering created from the input parameters.
     *
     * Example:
     * @code
     * TTL_event import_DB_e = TTL_get_event();
     * TTL_import_double_buffering import_db(l_in1, l_in2, ext_base_in, ext_layout_in, &import_DB_e);
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
     */
    TTL_import_double_buffering(TTL_local(TENSORTYPE *) int_base1, TTL_local(TENSORTYPE *) int_base2,
                                TTL_tensor<TENSORTYPE> ext_tensor, TTL_event *event, TTL_tile first_tile) {
        m_common.int_base[0] = int_base1;
        m_common.int_base[1] = int_base2;

        m_common.ext_tensor_in = ext_tensor;
        m_event = event;
        m_common.index = 0;

        m_prev_tile = TTL_tile();

        step_buffering(first_tile);
    }

    TTL_sub_tensor<TENSORTYPE> step_buffering(const TTL_tile next_tile) {
        // For performance, compute everything possible before waiting for the
        // previous operations to finish.
        const TTL_layout int_layout(next_tile.shape.width, next_tile.shape.height);
        const TTL_sub_tensor<TENSORTYPE> import_to(
            m_common.int_base[m_common.index], next_tile.shape, int_layout, m_common.ext_tensor_in, next_tile.offset);

        const TTL_tensor<TENSORTYPE> import_from(m_common.ext_tensor_in.base,
                                                 next_tile.shape,
                                                 m_common.ext_tensor_in.layout,
                                                 next_tile.offset,
                                                 m_common.ext_tensor_in.elem_size);

        TTL_wait(1, m_event);

        if (next_tile.empty() == false) {
            TTL_import_sub_tensor(import_to, import_from, m_event);
        }

        m_common.index = (m_common.index + 1) % 2;

        const TTL_layout prev_int_layout(m_prev_tile.shape.width, m_prev_tile.shape.height);
        const TTL_sub_tensor<TENSORTYPE> result(m_common.int_base[m_common.index],
                                                m_prev_tile.shape,
                                                prev_int_layout,
                                                m_common.ext_tensor_in,
                                                m_prev_tile.offset);

        m_prev_tile = next_tile;

        return result;
    }

    /**
     * @brief Complete any transfers required to finish the buffering process.
     *
     * Any transfers that are still in progress will be completed and any transfers
     * that need to be started and completed before finish_buffering returns
     */
    void finish_buffering() {
        // Nothing to do.
    }

    TTL_event *m_event;    ///< A pointer to the event that is used to
                           ///< track the progress of the transfer
    TTL_tile m_prev_tile;  ///< Store of the previous imported tile */

    TTL_common_buffering<TENSORTYPE, 2> m_common;  ///< The information that is m_common to all pipeline schemes
};

template <typename TENSORTYPE>
struct TTL_export_double_buffering {
    /**
     * @brief Create a TTL_import_double_buffering and begin the buffering process
     *
     * @param int_base1 A pointer to the 1st local buffer
     * @param int_base2 A pointer to the 2nd local buffer
     * @param ext_tensor A tensor describing the input in global memory
     * @param event A pointer to the event to use for the inward (external to
     * internal) transfer completion
     *
     * @return The TTL_export_double_buffering created from the input parameters.
     *
     * Example:
     * @code
     * TTL_event import_DB_e = TTL_get_event();
     * TTL_export_double_buffering import_db(
     *       l_in1, l_in2, ext_base_in, ext_layout_in, &import_DB_e);
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
     */
    TTL_export_double_buffering(TTL_local(TENSORTYPE *) int_base1, TTL_local(TENSORTYPE *) int_base2,
                                TTL_tensor<TENSORTYPE> ext_tensor, TTL_event *event) {
        m_common.int_base[0] = int_base1;
        m_common.int_base[1] = int_base2;

        m_common.ext_tensor_in = ext_tensor;
        m_event = event;
        m_common.index = 0;

        m_prev_tile = TTL_tile();
    }

    /**
     * @brief Wait for the previous import operation to complete before beginning an
     * export of next tile.
     *
     * @param tile_current A description of the tile to begin exporting.
     *
     */
    TTL_sub_tensor<TENSORTYPE> step_buffering(TTL_tile tile_current) {
        const TTL_layout int_layout(m_prev_tile.shape.width, m_prev_tile.shape.height);
        const TTL_tensor export_from(
            m_common.int_base[m_common.index], m_prev_tile.shape, int_layout, m_common.ext_tensor_in.elem_size);
        const TTL_tensor export_to(m_common.ext_tensor_in.base,
                                   m_prev_tile.shape,
                                   m_common.ext_tensor_in.layout,
                                   m_prev_tile.offset,
                                   m_common.ext_tensor_in.elem_size);

        TTL_wait(1, m_event);

        if (m_prev_tile.empty() == false) TTL_export(export_from, export_to, m_event);

        m_common.index = (m_common.index + 1) % 2;  // TTL_ARRAYSIZE(m_common.int_base);
        const TTL_layout curr_int_layout(tile_current.shape.width, tile_current.shape.height);
        const TTL_sub_tensor result(m_common.int_base[m_common.index],
                                    tile_current.shape,
                                    curr_int_layout,
                                    m_common.ext_tensor_in,
                                    tile_current.offset);
        m_prev_tile = tile_current;

        return result;
    }

    /**
     * @brief Complete any transfers required to finish the buffering process.
     *
     * Any transfers that are still in progress will be completed and any transfers
     * that need to be started and completed before finish_buffering returns
     */
    void finish_buffering() {
        step_buffering(TTL_tile());
        step_buffering(TTL_tile());
    }

    TTL_event *m_event;    ///< A pointer to the event that is used to
                           ///< track the progress of the transfer
    TTL_tile m_prev_tile;  ///< Store of the previous imported tile */

    TTL_common_buffering<TENSORTYPE, 2> m_common;  ///< The information that is m_common to all pipeline schemes
};