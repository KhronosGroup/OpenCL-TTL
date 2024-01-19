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
    for (int y = 0; y < tensor_out.tensor.shape.height; ++y) {
        for (int x = 0; x < tensor_out.tensor.shape.width; ++x) {
            const int x_in = x + TILE_OVERLAP_LEFT;
            const int y_in = y + TILE_OVERLAP_TOP;
            const TEST_TENSOR_TYPE left = TTL_read_tensor(tensor_in, x_in - 1, y_in);
            const TEST_TENSOR_TYPE above = TTL_read_tensor(tensor_in, x_in, y_in - 1);
            const TEST_TENSOR_TYPE centre = TTL_read_tensor(tensor_in, x_in, y_in);
            const TEST_TENSOR_TYPE right = TTL_read_tensor(tensor_in, x_in + 1, y_in);
            const TEST_TENSOR_TYPE bottom = TTL_read_tensor(tensor_in, x_in, y_in + 1);

            TTL_write_tensor(tensor_out, left + above + centre + right + bottom, x, y);
        }
    }
}
