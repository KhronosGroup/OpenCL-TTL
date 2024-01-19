/*
 * TTL_types.h
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

#include "stdbool.h"  // Define bool, false, and true
#include "stddef.h"
#include "stdio.h"  // Include to make printf appear for other include files

typedef unsigned char uchar;    ///< opencl and so TTL supports a type called uchar which is not part of C
#define __global                ///< The opencl __global namespace is not supported in C
#define __local                 ///< The opencl __local namespace is not supported in C
typedef unsigned char event_t;  ///< event_t is not supported, so provide a harmless placeholder
typedef unsigned char uchar;    ///< OpenCL supports uchar so provide the same in c
typedef unsigned int uint;      ///< OpenCL supports uint so provide the same in c
typedef unsigned short ushort;  ///< OpenCL supports ushort so provide the same in c
typedef unsigned long ulong;    ///< OpenCL supports ulong so provide the same in c

#include "../opencl/TTL_types.h"
