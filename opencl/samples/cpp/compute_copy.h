/*
 * compute_cross.h
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

#include "TTL/TTL.h"

#define TILE_OVERLAP_LEFT 0
#define TILE_OVERLAP_RIGHT 0
#define TILE_OVERLAP_TOP 0
#define TILE_OVERLAP_BOTTOM 0

void compute(TTL_int_sub_tensor_t tensor_in, TTL_int_sub_tensor_t tensor_out) {
    __local const unsigned char* const l_in = tensor_in.tensor.base;
    __local unsigned char* const l_out = tensor_out.tensor.base;
    const int width = tensor_out.tensor.shape.width;
    const int height = tensor_out.tensor.shape.height;
    const int stride_in = tensor_in.tensor.layout.row_spacing;
    const int stride_out = tensor_out.tensor.layout.row_spacing;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            l_out[(y * stride_out) + x] = l_in[(y * stride_in) + x];
        }
    }
}