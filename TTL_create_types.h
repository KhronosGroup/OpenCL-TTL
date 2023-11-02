/*
 * TTL_create_types.h
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

/************************************************************************************************************
 * We genrate sizeof_TYPE to allow for sizeof anything - deals with sizeof(void)
 ************************************************************************************************************/

#pragma push_macro("TTL_TENSOR_TYPE")

#ifndef TTL_TYPES_NO_VOID
#define TTL_TENSOR_TYPE void
#define sizeof_void sizeof(char)
#include "TTL_create_type.h"
#endif
#undef TTL_TYPES_NO_VOID

#define TTL_TENSOR_TYPE char
#define sizeof_char sizeof(char)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE uchar
#define sizeof_uchar sizeof(uchar)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE int
#define sizeof_int sizeof(int)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE uint
#define sizeof_uint sizeof(uint)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE short
#define sizeof_short sizeof(short)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE ushort
#define sizeof_ushort sizeof(ushort)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE long
#define sizeof_long sizeof(long)
#include "TTL_create_type.h"

#define TTL_TENSOR_TYPE ulong
#define sizeof_ulong sizeof(ulong)
#include "TTL_create_type.h"

#undef TTL_TYPES_INCLUDE_FILE

#pragma pop_macro("TTL_TENSOR_TYPE")
