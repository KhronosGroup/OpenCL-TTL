/*
 * main.c
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
#include <stdint.h>

#ifndef TENSOR_WIDTH
constexpr uint32_t TENSOR_WIDTH = 103;
constexpr uint32_t TENSOR_HEIGHT = 27;
constexpr uint32_t EXTERNAL_STRIDE_IN = 150;
constexpr uint32_t EXTERNAL_STRIDE_OUT = 103;
#endif
constexpr uint32_t TILE_WIDTH = 44;
constexpr uint32_t TILE_HEIGHT = 33;
