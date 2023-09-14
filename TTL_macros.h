/*
 * TTL_macros.h
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

/**
 * @def NO_PARAMETERS
 *
 * Sometimes tools require a void function to be defined as
 *
 * Function() not Function(void) - the later is correct but this
 * macro allows for it to be overridden.
 */
#ifndef __TTL_NO_PARAMETERS
#define __TTL_NO_PARAMETERS void
#endif

#if __TTL_DEBUG > 0
#define __TTL_TRACE_FN(func, ...) func(__VA_ARGS__, unsigned int line)
#define __TTL_TRACE_LINE , line

#else
/**
 * @def __TTL_TRACE_FN
 *
 * A function defined using __TTL_TRACE_FN will have an additional "unsigned int line" parameter
 * added if __TTL_DEBUG > 0.  This line parameter allows for the output of the line of code originating
 * the call.
 *
 * For example:
 * static inline TTL_int_sub_tensor_t __TTL_TRACE_FN(TTL_step_buffering, TTL_import_double_buffering_t *const dbi,
 *                                                  const TTL_tile_t next_tile);
 *
 * Will produce
 *  static inline TTL_int_sub_tensor_t TTL_step_buffering(TTL_import_double_buffering_t *const dbi,
 *                                                  const TTL_tile_t next_tile);
 * when __TTL_DEBUG == 0
 *  static inline TTL_int_sub_tensor_t TTL_step_buffering(TTL_import_double_buffering_t *const dbi,
 *                                                  const TTL_tile_t next_tile, unsinged int line);
 * when __TTL_DEBUG > 0
 */
#define __TTL_TRACE_FN(func, ...) func(__VA_ARGS__)

/**
 * @def __TTL_TRACE_LINE
 *
 * Is used internally to TLL to append the __LINE__ to function calls when __TTL_DEBUG > 0
 *
 * TTL_step_buffering(dbi, next_tile __TTL_TRACE_LINE);
 *
 * The file TTL_trace_macros deals with this for calls post TTL.h
 */
#define __TTL_TRACE_LINE

#endif


/************************************************************************************************************
 * Generic name generator
 ***********************************************************************************************************/
/**
 * @def __TTL_tensor_name
 *
 * @brief Generate a generic "[prefix][const]_[ext, int]_[type]_[sub]_tensor[suffix]" style name from the elements
 *
 * @param prefix The prefixed to apply to the name
 * @param const_2 The const name to place after the prefix - should be empty or const_
 * @param location The location of the tensor - should be ext or int
 * @param type The type of the tensor - should be any valid c type
 * @param sub If the tensor is a sub tensor or not - should be empty or sub_
 * @param suffix The suffix of to apply to the name
 *
 * @details
 * __TTL_tensor_name(TTL_create, const_, ext, char, sub_, ) will give TTL_create_const_ext_char_sub_tensor
 * __TTL_tensor_name(TTL_, ,int, void, , _t) will give TTL_int_void_tensor_t
 *
 * The 'double' call is some 'magic' to allow the caller itself to contain a macro.
 * 
 * @return The generated name
 */
#define __TTL_tensor_name_1(prefix, const_1, location, type, sub, suffix) \
    prefix##const_1##location##type##_##sub##tensor##suffix
#define __TTL_tensor_name(prefix, const_1, location, type, sub, suffix) \
    __TTL_tensor_name_1(prefix, const_1, location, type, sub, suffix)

#define __TTL_tensor_overloaded_name_1(prefix, const_1, location, type, sub, suffix) \
    prefix##const_1##location##sub##tensor##suffix
#define __TTL_tensor_overloaded_name(prefix, const_1, location, type, sub, suffix) \
    __TTL_tensor_overloaded_name_1(prefix, const_1, location, type, sub, suffix)

/**
 * @def __TTL_tensor_no_type_name
 *
 * @brief Generate a generic "[prefix][const]_[ext, int]_[sub]_tensor[suffix]" style name from the elements
 *
 * @param prefix The prefixed to apply to the name
 * @param const_2 The const name to place after the prefix - should be empty or const_
 * @param location The location of the tensor - should be ext or int
 * @param sub If the tensor is a sub tensor or not - should be empty or sub_
 * @param suffix The suffix of to apply to the name
 *
 * @details
 * __TTL_tensor_no_type_name(TTL_create, const_, ext, sub_, ) will give TTL_create_const_ext_sub_tensor
 * __TTL_tensor_no_type_name(TTL_, ,int, , _t) will give TTL_int_tensor_t
 *
 * The 'double' call is some 'magic' to allow the caller itself to contain a macro.
 * 
 * @return The generated name
 */
#define __TTL_tensor_no_type_name_1(prefix, const_1, location, sub, suffix) \
    prefix##const_1##location##sub##tensor##suffix
#define __TTL_tensor_no_type_name(prefix, const_1, location, sub, suffix) \
    __TTL_tensor_no_type_name_1(prefix, const_1, location, sub, suffix)
