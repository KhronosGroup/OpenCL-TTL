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

/**
 * This file is used for the preprocessed file header case.
 *
 * It selected TTL_c.h or TTL_opencl.h depending on the value of
 * TTL_TARGET
 *
 * Durring an install with TTL_PRE_GENERATE it will be placed in the
 * target directory alongside TTL_c.h and TTL_opencl.h
 */

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
 * @def __TTL_STR_CONCAT1
 *
 * Concatenate x and y, must be used within another macro.
 */
#define __TTL_STR_CONCAT1(x, y) x##y

/**
 * @def __TTL_STR_CONCAT2
 *
 * Concatenate x and y, can be used standalone
 */
#define __TTL_STR_CONCAT2(x, y) __TTL_STR_CONCAT1(x, y)

/**
 * @def __TTL_STRINGFY1
 *
 * Turn s into a "string", must be used within another macro
 */
#define __TTL_STRINGFY1(s) #s

/**
 * @def __TTL_STRINGFY2
 *
 * Turn s into a "string", can be used standalone
 */
#define __TTL_STRINGFY2(s) __TTL_STRINGFY1(s)

/**
 * @def TTL_INCLUDE_H
 *
 * @brief Create an include name based on TTL_TARGET
 *
 * Values in the base distribution include
 *   - opencl/TTL_types.h
 *   - c/TTL_types.h
 */
// clang-format off
#define TTL_INCLUDE_H_1 __TTL_STR_CONCAT2(TTL_, TTL_TARGET)
#define TTL_INCLUDE_H_2 __TTL_STRINGFY2(TTL_INCLUDE_H_1.h)
// clang-format on

#include TTL_INCLUDE_H_2
