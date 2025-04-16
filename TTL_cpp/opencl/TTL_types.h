/*
 * TTL_types.h
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

#include <stdint.h>

using TTL_dim = uint32_t;  ///< The type used to hold the size of an object along any dimension
using TTL_offset_dim = int32_t;    ///< The type used to hold offsets and origins.

/**
 * @def TTL_global
 * @brief Create a typed reference in the __global address space.
 *
 * Using this macro allows a global reference to be created in an implmentation indepedent way.
 *
 * For example a pointer to a struct MyStruct in the global namespace.
 *
 * @code {.c}
 * TTL_global(MyStruct *)
 * @endcode
 *
 * for an unsigned int
 *
 * @code {.c}
 * TTL_global(unsigned int *)
 * @endcode
 */
#define TTL_global(type) __global type

/**
 * @def TTL_global_printf
 *
 * Different implementations use different types for global pointers, using TTL_global_printf
 * allows for agnostic implementations.
 *
 * @code {.c}
 * TTL_global(MyStruct *) ptr_my_struct;
 * printf("Ptr to my struct " TTL_global_printf "\n", ptr_my_struct)
 * @endcode
 */
#define TTL_global_printf "%p"

/**
 * @def TTL_local
 * @brief Create a typed reference in the __local address space.
 *
 * Using this macro allows a local reference to be created in an implmentation indepedent way.
 *
 * For example a pointer to a struct MyStruct in the local namespace.
 *
 * @code {.c}
 * TTL_local(MyStruct *)
 * @endcode
 *
 * for an unsigned int
 *
 * @code {.c}
 * TTL_local(unsigned int *)
 * @endcode
 */
#define TTL_local(type) __local type

/**
 * @def TTL_local_printf
 *
 * Different implementations use different types for local pointers, using TTL_local_printf
 * allows for agnostic implementations.
 *
 * @code {.c}
 * TTL_local(MyStruct *) ptr_my_struct;
 * printf("Ptr to my struct " TTL_local_printf "\n", ptr_my_struct)
 * @endcode
 */
#define TTL_local_printf "%p"

/**
 * @brief TTL_event is a pseudonym for OpenCL event_t
 *
 * To allow full compatibility with OpenCL but allow other implementations to
 * use TTL_event in a way that is more applicable to their platforms we
 * used TTL_event.  For OpenCL TTL_event is event_t.
 */
using TTL_event = event_t;

/**
 * @brief Internal non-API helper function to allow debugging of events
 *
 * @param event
 *
 * @todo Add file name information if/once PHDL supports %s in format strings
 */

static inline void __TTL_dump_event(const TTL_event* const ttl_event) {
    // In OpenCL compiler an event_t is a pointer to a mask of channels
    const unsigned char* const event = *(const unsigned char**)ttl_event;

    if (!event) {
        printf("event=NULL");
    } else {
        printf("event=%p (channels mask=0x%x)", event, *event);
    }
}
