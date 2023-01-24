/*
 * TTL.h
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

#include "TTL_macros.h"

/**
 * @def __TTL_DEBUG
 *
 * @brief Turn on printf outputs from TTL, can be very noisy
 *
 * 0 = no noise
 * bigger numbers mean more noise.
 */
#ifndef __TTL_DEBUG
#define __TTL_DEBUG 0
#endif

/**
 * @def TTL_TARGET
 *
 * @brief Define the target for TTL
 *
 * TTL can be built for multible targets - native support is
 *   - opencl - default if TTL_TARGET not predefined.
 *   - c
 *
 * Other platforms can be provided.
 */
#ifndef TTL_TARGET
#define TTL_TARGET opencl
#endif

/**
 * @def TTL_TYPES_INCLUDE_H
 *
 * @brief TTL_types will include this file prior to its definitions
 *
 * Values in the base distribution include
 *   - opencl/TTL_types.h
 *   - c/TTL_types.h
 */
// clang-format off
#define TTL_TYPES_INCLUDE_H __TTL_STRINGFY2(TTL_TARGET/TTL_types.h)
// clang-format on

/**
 * @def TTL_IMPORT_EXPORT_INCLUDE_H
 *
 * @brief Allow override of the standard OpenCL import export rules
 *
 *  * Values in the base distribution include
 *   - opencl/TTL_import_export.h
 *   - c/TTL_import_export.h
 */
// clang-format off
#define TTL_IMPORT_EXPORT_INCLUDE_H __TTL_STRINGFY2(TTL_TARGET/TTL_import_export.h)
// clang-format on

#include "TTL_pipeline_schemes.h"
#include "TTL_trace_macros.h"