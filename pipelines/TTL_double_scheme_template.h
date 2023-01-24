/*
 * TTL_double_scheme_template.h
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
 * TTL_double_buffering pipelines a duplex import or export transaction using two
 * internal buffers.
 * 
 * The following table draws the pipelined actions performed in double buffering.
 * It specifies which tile is processed in each iteration:
 * 
 * | Action\\Iteration | \#-1 | \#0 | \#1 | \#2 | \#i (2:NumOfTiles-2) | \#NumOfTiles-1 | \#NumOfTiles | \#NumOfTiles+1 |
 * |-------------------|------|-----|-----|-----|----------------------|----------------|--------------|----------------|
 * | **Wait Import**   |      | 0   | 1   | 2   | i                    | NumOfTiles-1   |              |                |
 * | **Import**        | 0    | 1   | 2   | 3   | i+1                  |                |              |                |
 * | **WaitExport**    |      |     |     | 0   | i-2                  | NumOfTiles-3   | NumOfTiles-2 | NumOfTiles-1   |
 * | **Export**        |      |     | 0   | 1   | i-1                  | NumOfTiles-2   | NumOfTiles-1 |                |
 * | **Compute**       |      | 0   | 1   | 2   | i                    | NumOfTiles-1   |              |                |
 * 
 * Notice the prolog (at iteration number -1) and the 2 epilogs (at iterations
 * number NumOfTiles and NumOfTiles+1) which add in total 3 extra iterations.
 *
 * @example TTL_double_buffering.cl
 */
// clang-format on

// This file presumes that the following have been pre included.
// this is not done here for path reasons.
// #include "TTL_core.h"
// #include "TTL_import_export.h"
// #include TTL_IMPORT_EXPORT_INCLUDE_H
#include "../TTL_macros.h"
#include "TTL_schemes_common.h"

#define TTL_JOIN(a, b, c) a##b##c
#ifdef TTL_IMPORT_DOUBLE
#define TTL_name(prefix, suffix) TTL_JOIN(prefix, _import_, suffix)
#define TTL_double_buffering_internal_t TTL_name(TTL, double_buffering_t)
#define TTL_ext_tensor_type TTL_const_ext_tensor_t
#define TTL_int_ptr void *
#endif

#ifdef TTL_EXPORT_DOUBLE
#define TTL_name(prefix, suffix) TTL_JOIN(prefix, _export_, suffix)
#define TTL_double_buffering_internal_t TTL_name(TTL, double_buffering_t)
#define TTL_ext_tensor_type TTL_ext_tensor_t
#define TTL_int_ptr void *
#endif

/**
 * @brief Data required to perform double buffer pipelining.
 *
 * @see TTL_start_import_double_buffering and
 * TTL_start_export_double_buffering for a description of double buffer
 * pipelining.
 */
typedef struct {
    TTL_common_buffering_t(
        TTL_int_ptr, TTL_ext_tensor_type) common;  ///< @brief The information that is common to all pipeline schemes

    TTL_event_t *event;

    TTL_tile_t prev_tile; /** @brief Store of the previous imported/exported tile */
} TTL_double_buffering_internal_t;

#ifdef TTL_IMPORT_DOUBLE
/**
 * @brief Create a TTL_import_double_buffering_t and begin the buffering process
 *
 * @param int_base1 A pointer to the 1st local buffer
 * @param int_base2 A pointer to the 2nd local buffer
 * @param ext_tensor_in A tensor describing the input in global memory
 * @param event A pointer to the event to use for the inward (external to
 * internal) transfer completion
 * @param first_tile The first tile to fetch for the scheme
 *
 * @return The TTL_import_double_buffering_t created from the input parameters.
 *
 * Example:
 * @code
 * TTL_event_t import_DB_e = TTL_get_event();
 * TTL_import_double_buffering_t import_db = TTL_start_import_double_buffering(
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
static inline TTL_double_buffering_internal_t TTL_start_import_double_buffering(TTL_local(TTL_int_ptr) int_base1,
                                                                                TTL_local(TTL_int_ptr) int_base2,
                                                                                TTL_ext_tensor_type ext_tensor,
                                                                                TTL_event_t *event,
                                                                                TTL_tile_t first_tile);
#endif

#ifdef TTL_EXPORT_DOUBLE
/**
 * @brief Create a TTL_export_double_buffering_t and begin the buffering process
 *
 * @param int_base1 A pointer to the 1st local buffer
 * @param int_base2 A pointer to the 2nd local buffer
 * @param ext_tensor_out  A tensor describing the output in global memory
 * @param event A pointer to the event to use for the inward and outward
 * transfer completion
 *
 * Solid description of single buffering here.
 *
 * @return The TTL_export_double_buffering_t created from the input parameters.
 *
 * Example:
 * @code
 * TTL_event_t export_DB_e = TTL_get_event();
 * TTL_export_double_buffering_t import_db = TTL_start_export_double_buffering(
 *       l_in1, l_in2, ext_base_in, ext_layout_in, &export_DB_e);
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
static inline TTL_double_buffering_internal_t TTL_start_export_double_buffering(TTL_local(TTL_int_ptr) int_base1,
                                                                                TTL_local(TTL_int_ptr) int_base2,
                                                                                TTL_ext_tensor_type ext_tensor,
                                                                                TTL_event_t *event);
#endif

static inline TTL_int_sub_tensor_t __attribute__((overloadable))__TTL_TRACE_FN(TTL_step_buffering, TTL_import_double_buffering_t *const dbi,
                                                  const TTL_tile_t next_tile);

static inline TTL_double_buffering_internal_t __TTL_TRACE_FN(TTL_name(TTL_start, double_buffering),
                                                             TTL_local(TTL_int_ptr) int_base1,
                                                             TTL_local(TTL_int_ptr) int_base2,
                                                             TTL_ext_tensor_type ext_tensor, TTL_event_t *event
#ifdef TTL_IMPORT_DOUBLE
                                                             ,
                                                             TTL_tile_t first_tile
#endif
) {
    TTL_double_buffering_internal_t result;

    result.common.int_base[0] = int_base1;
    result.common.int_base[1] = int_base2;

    result.common.ext_tensor_in = ext_tensor;
    result.event = event;
    result.common.index = 0;

    result.prev_tile = TTL_create_empty_tile();

#ifdef TTL_IMPORT_DOUBLE
    TTL_step_buffering(&result, first_tile __TTL_TRACE_LINE);
#endif

    return result;
}

// Clear up the mess we made!
#undef TTL_JOIN
#undef TTL_IMPORT_DOUBLE
#undef TTL_EXPORT_DOUBLE
#undef TTL_name
#undef TTL_double_buffering_internal_t
#undef TTL_int_ptr
#undef TTL_ext_tensor_type
