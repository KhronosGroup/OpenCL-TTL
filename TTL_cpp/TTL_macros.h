/*
 * TTL_macros.h
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
 * @def TTL_ARRAYSIZE
 *
 * @brief Return the number of elements in the array x
 *
 * @param x The array to return the size of
 */
#define TTL_ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))
