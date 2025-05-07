/*
 * TTL_import_export.h
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

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief TTL_event is a pseudonym for OpenCL event_t
 *
 * To allow full compatibility with OpenCL but allow other implementations to
 * use TTL_event in a way that is more applicable to their platforms we
 * used TTL_event.  For OpenCL TTL_event is event_t.
 */
typedef event_t TTL_event;

/**
 * @brief Wait for events that identify the async_work_group_copy operations to
 * complete.
 *
 * @param num_events Number of events to wait for (size of event_list)
 * @param event_list A pointer to a list of events.
 *
 * Not supported in the 'C' version.
 *
 * @see OpenCL's wait_group_events() builtin for more information.
 */
static inline void wait_group_events(int num_events, event_t *event_list) {
    (void)num_events;
    (void)event_list;
    // Nothing to do in C we have no events.
}

static inline event_t async_work_group_copy_3D3D(void *const dst, size_t dst_offset, const void *const src,
                                                 size_t src_offset, size_t num_bytes_per_element,
                                                 size_t num_elements_per_line, size_t num_lines, size_t num_planes,
                                                 size_t src_total_line_length, size_t src_total_plane_spacing,
                                                 size_t dst_total_line_length, size_t dst_total_plane_spacing,
                                                 event_t event) {
    (void)dst_total_line_length;
    (void)event;

    for (size_t plane = 0; plane < num_planes; plane++) {
        const uchar *src_ptr =
            (uchar *)src + ((src_offset + (src_total_plane_spacing * plane)) * num_bytes_per_element);
        uint8_t *dst_ptr = (uint8_t *)dst + ((dst_offset + (dst_total_plane_spacing * plane)) * num_bytes_per_element);

        for (size_t line = 0; line < num_lines; line++) {
            memcpy(dst_ptr, src_ptr, num_bytes_per_element * num_elements_per_line);

            src_ptr += src_total_line_length * num_bytes_per_element;
            dst_ptr += dst_total_line_length * num_bytes_per_element;
        }
    }

    return event;
}

#include "../opencl/TTL_import_export.h"
