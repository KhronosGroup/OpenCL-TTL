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

#include <stdbool.h>

#include "TTL/TTL.h"

#define TILE_OVERLAP_LEFT 1
#define TILE_OVERLAP_RIGHT 1
#define TILE_OVERLAP_TOP 1
#define TILE_OVERLAP_BOTTOM 1

template <typename TENSORTYPE>
void compute(TTL_sub_tensor<TENSORTYPE> tensor_in, TTL_sub_tensor<TENSORTYPE> tensor_out) {
    for (TTL_dim_t y = 0; y < tensor_out.tensor.shape.height; ++y) {
        for (TTL_dim_t x = 0; x < tensor_out.tensor.shape.width; ++x) {
            const int x_in = x + TILE_OVERLAP_LEFT;
            const int y_in = y + TILE_OVERLAP_TOP;

            if (true) {
                const TENSORTYPE left = tensor_in.read(x_in - 1, y_in);
                const TENSORTYPE above = tensor_in.read(x_in, y_in - 1);
                const TENSORTYPE centre = tensor_in.read(x_in, y_in);
                const TENSORTYPE right = tensor_in.read(x_in + 1, y_in);
                const TENSORTYPE bottom = tensor_in.read(x_in, y_in + 1);
                tensor_out.write(left + above + centre + right + bottom, x, y);
            } else {
                const TENSORTYPE centre = tensor_in.read(x_in, y_in);
                tensor_out.write(centre, x, y);
            }
        }
    }
}

template <typename TENSORTYPE>
bool result_check(TENSORTYPE* const ext_base_in, TENSORTYPE* const ext_base_out, const int width, const int height,
                  const int tile_width, const int tile_height) {
#define input_buffer ((TENSORTYPE(*)[height][width])ext_base_in)
#define output_buffer ((TENSORTYPE(*)[height][width])ext_base_out)

    // TENSORTYPE(* input_buffer)[height][width] = (TENSORTYPE(*)[height][width])ext_base_in;
    // TENSORTYPE(* output_buffer)[height][width] = (TENSORTYPE(*)[height][width])ext_base_out;
    bool result = true;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            TENSORTYPE expected = input_buffer[0][y][x];

            if (true) {
                if (x > 0) expected += input_buffer[0][y][x - 1];
                if (y > 0) expected += input_buffer[0][y - 1][x];
                if (x < (width - 1)) expected += input_buffer[0][y][x + 1];
                if (y < (height - 1)) expected += input_buffer[0][y + 1][x];
            }

            if (output_buffer[0][y][x] != expected) {
                printf("Mismatch at [%d, %d]  %d  != %d Tensor size[% d, % d], Tile size[% d, % d]\n ",
                       x,
                       y,
                       output_buffer[0][y][x],
                       expected,
                       width,
                       height,
                       tile_width,
                       tile_height);
                result = false;
            }
        }
    }

    return result;
}
