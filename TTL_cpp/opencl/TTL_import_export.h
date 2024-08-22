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
 * async_work_group_copy_3D3D is not supported by all OpenCL drivers
 * including some V3.0 drivers. To resolve this define TTL_COPY_3D
 */
#include "TTL_async_work_group_copy_3D3D.h"
#endif

/**
 * @brief Return an empty event of type TTL_event
 *
 * @see TTL_event for information about how event_t and TTL_event relate
 *
 * @return The return value allows an empty event to be passed to APIs that
 * require an event and return/update the value with a new event value.
 */
static inline TTL_event TTL_get_event() {
    return (event_t)0;
}

/**
 * @def TTL_wait
 *
 * Wait for the array of events passed to enter the complete state.
 */
static inline void TTL_wait(const int num_events, TTL_event *const events) {
    __TTL_dump_wait(num_events, events, __LINE__);

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
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
void TTL_import_base(const TTL_tensor<INT_TENSORTYPE> &internal_tensor,
                     const TTL_tensor<EXT_TENSORTYPE> &external_tensor, TTL_event *const event) {
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

    __TTL_dump_transaction(false, internal_tensor, external_tensor, 0, event, __LINE__);
}

/**
 * @brief  Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tensor.
 * @param external_tensor A TTL_int_tensor_t describing the external tensor.
 * Complete description of what not how here.
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
void TTL_blocking_import_base(const TTL_tensor<INT_TENSORTYPE> &internal_tensor,
                              const TTL_tensor<EXT_TENSORTYPE> &external_tensor) {
    TTL_event event = TTL_get_event();
    TTL_import_base(internal_tensor, external_tensor, &event);
    TTL_wait(1, &event);
}

/**
 * @brief Begin the asynchronous export of the external tensor to the internal tensor
 *
 * @param internal_tensor internal_tensor A TTL_int_tensor_t describing the internal tile.
 * @param external_tensor external_tensor A TTL_int_sub_tensor_t describing the external tile.
 * @param event event_ptr A pointer to the event which describe the transfer.
 *
 * @note async_work_group_copy_3D3D is not supported by all OpenCL drivers
 * including some V3.0 drivers. To resolve this define TTL_COPY_3D
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
static inline void TTL_export_base(const TTL_tensor<INT_TENSORTYPE> internal_tensor,
                                   const TTL_tensor<EXT_TENSORTYPE> external_tensor, TTL_event *const event) {
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

    __TTL_dump_transaction(true, internal_tensor, external_tensor, 0, event, __LINE__);
}

/**
 * @brief Export the external tensor to the internal tensor returning when complete
 *
 * @param internal_tensor A TTL_int_tensor_t describing the internal tile.
 * @param external_tensor A TTL_int_sub_tensor_t describing the external tile.
 *
 * Complete description of what not how here.
 */
template <typename INT_TENSORTYPE, typename EXT_TENSORTYPE>
static inline void TTL_blocking_export_base(const TTL_tensor<INT_TENSORTYPE> internal_tensor,
                                            const TTL_tensor<EXT_TENSORTYPE> external_tensor) {
    TTL_event event = TTL_get_event();
    TTL_export_base(internal_tensor, external_tensor, &event);
    TTL_wait(1, &event);
}
