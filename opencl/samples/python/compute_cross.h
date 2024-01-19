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

#define TILE_OVERLAP_LEFT 1
#define TILE_OVERLAP_RIGHT 1
#define TILE_OVERLAP_TOP 1
#define TILE_OVERLAP_BOTTOM 1

void compute(__TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, sub_, _t) tensor_in,
             __TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, sub_, _t) tensor_out) {
    __local const TEST_TENSOR_TYPE* const l_in = tensor_in.tensor.base;
    __local TEST_TENSOR_TYPE* const l_out = tensor_out.tensor.base;

    const int x_shift = tensor_out.origin.sub_offset.x - tensor_in.origin.sub_offset.x;
    const int y_shift = tensor_out.origin.sub_offset.y - tensor_in.origin.sub_offset.y;
    const int width = tensor_out.tensor.shape.width;
    const int height = tensor_out.tensor.shape.height;
    const int stride_in = tensor_in.tensor.layout.row_spacing;
    const int stride_out = tensor_out.tensor.layout.row_spacing;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int x_in = x + x_shift;
            const int y_in = y + y_shift;
            const int left = (y_in * stride_in) + (x_in - 1);
            const int above = ((y_in - 1) * stride_in) + x_in;
            const int centre = (y_in * stride_in) + x_in;
            const int right = (y_in * stride_in) + (x_in + 1);
            const int bottom = ((y_in + 1) * stride_in) + x_in;

            l_out[y * stride_out + (x)] = l_in[left] + l_in[above] + l_in[centre] + l_in[right] + l_in[bottom];
        }
    }
}