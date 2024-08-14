/*
 * TTL_import_export.h
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

#include "../TTL_macros.h"

#ifdef TTL_COPY_3D
/*
 * async_work_group_copy_3D3D if not supported by all OpenCL drivers
 * including some V3.0 drivers. To resolve this define TTL_COPY_3D
 */
#include "TTL_async_work_group_copy_3D3D.h"
#endif

/**
 * @brief Return an empty event of type TTL_event_t
 *
 * @see TTL_event_t for information about how event_t and TTL_event_t relate
 *
 * @return The return value allows an empty event to be passed to APIs that
 * require an event and return/update the value with a new event value.
 */
static inline TTL_event_t TTL_get_event() {
    return (event_t)0;
}

/**
 * @def TTL_wait
 *
 * Wait for the array of events passed to enter the complete state.
 *
 * @return No return value
 */
static inline void __TTL_TRACE_FN(TTL_wait, const int num_events, TTL_event_t *const events) {
#if __TTL_DEBUG > 0
    __TTL_dump_wait(num_events, events __TTL_TRACE_LINE);
#endif  // __TTL_DEBUG

    wait_group_events(num_events, events);
}

/**
 * @brief TTL_import
 *
 * Begin the asynchronous import of the external tensor to the internal tensor
 *
 * @param internal_tensor internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor external_tensor A TTL_int_tensor_t describing the external tensor.
 * @param event event_ptr A pointer to the event which describe the transfer.
 *
 * @return No return value
 */
static inline void __attribute__((overloadable)) __TTL_TRACE_FN(TTL_import_base, const TTL_int_tensor_t internal_tensor,
                                  const TTL_const_ext_tensor_t external_tensor, TTL_event_t *event) {
    *event = async_work_group_copy_3D3D((__local void *)internal_tensor.base,
                                        0,
                                        (__global void *)external_tensor.base,
                                        0,
                                        internal_tensor.elem_size,
                                        internal_tensor.shape.width,
                                        internal_tensor.shape.height,
                                        internal_tensor.shape.depth,
                                        external_tensor.layout.row_spacing,
                                        external_tensor.layout.plane_spacing,
                                        internal_tensor.layout.row_spacing,
                                        internal_tensor.layout.plane_spacing,
                                        *event);

#if __TTL_DEBUG > 0
    __TTL_dump_transaction(
        false, TTL_to_const_tensor(&internal_tensor), &external_tensor, 0, event __TTL_TRACE_LINE);
#endif  // __TTL_DEBUG
}

/**
 * @brief  Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor A TTL_int_tensor_t describing the external tensor.
 * Complete description of what not how here.
 *
 * @return No return value
 */
static inline void  __TTL_TRACE_FN(TTL_blocking_import_base, const TTL_int_tensor_t internal_tensor,
                                  const TTL_const_ext_tensor_t external_tensor) {
    TTL_event_t event = TTL_get_event();
    TTL_import_base(internal_tensor, external_tensor, &event __TTL_TRACE_LINE);
    TTL_wait(1, &event __TTL_TRACE_LINE);
}

/**
 * @brief Begin the asynchronous export of the external tensor to the internal tensor
 *
 * @param internal_tensor internal_tensor A TTL_int_tensor_t describing the internal tile.
 * @param external_tensor external_tensor A TTL_int_sub_tensor_t describing the external tile.
 * @param event event_ptr A pointer to the event which describe the transfer.
 *
 * @note async_work_group_copy_3D3D if not supported by all OpenCL drivers
 * including some V3.0 drivers. To resolve this define TTL_COPY_3D
 *
 * @return No return value
 */
static inline void __TTL_TRACE_FN(TTL_export_base, const TTL_const_int_tensor_t internal_tensor,
                                  const TTL_ext_tensor_t external_tensor, TTL_event_t *const event) {
    *event = async_work_group_copy_3D3D((__global void *)external_tensor.base,
                                        0,
                                        (__local void *)internal_tensor.base,
                                        0,
                                        internal_tensor.elem_size,
                                        internal_tensor.shape.width,
                                        internal_tensor.shape.height,
                                        internal_tensor.shape.depth,
                                        internal_tensor.layout.row_spacing,
                                        internal_tensor.layout.plane_spacing,
                                        external_tensor.layout.row_spacing,
                                        external_tensor.layout.plane_spacing,
                                        *event);

#if __TTL_DEBUG > 0
    __TTL_dump_transaction(
        true, &internal_tensor, TTL_to_const_tensor(&external_tensor), 0, event __TTL_TRACE_LINE);
#endif  // __TTL_DEBUG
}

/**
 * @brief Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tile.
 * @param external_tensor A TTL_int_sub_tensor_t describing the external tile.
 *
 * Complete description of what not how here.
 *
 * @return No return value
 */
static inline void __TTL_TRACE_FN(TTL_blocking_export_base, const TTL_const_int_tensor_t internal_tensor,
                                  const TTL_ext_tensor_t external_tensor) {
    TTL_event_t event = TTL_get_event();
    TTL_export_base(internal_tensor, external_tensor, &event __TTL_TRACE_LINE);
    TTL_wait(1, &event __TTL_TRACE_LINE);
}
