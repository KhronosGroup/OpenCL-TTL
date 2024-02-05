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
    for (unsigned int y = 0; y < tensor_out.tensor.shape.height; ++y) {
        for (unsigned int x = 0; x < tensor_out.tensor.shape.width; ++x) {
            const int x_in = x + TILE_OVERLAP_LEFT;
            const int y_in = y + TILE_OVERLAP_TOP;
            const TEST_TENSOR_TYPE left = TTL_read_tensor(tensor_in, x_in - 1, y_in);
            const TEST_TENSOR_TYPE above = TTL_read_tensor(tensor_in, x_in, y_in - 1);
            const TEST_TENSOR_TYPE centre = TTL_read_tensor(tensor_in, x_in, y_in);
            const TEST_TENSOR_TYPE right = TTL_read_tensor(tensor_in, x_in + 1, y_in);
            const TEST_TENSOR_TYPE bottom = TTL_read_tensor(tensor_in, x_in, y_in + 1);

            if (true)
                TTL_write_tensor(tensor_out, left + above + centre + right + bottom, x, y);
            else
                TTL_write_tensor(tensor_out, centre, x, y);
        }
    }
}

bool result_check(TEST_TENSOR_TYPE* const ext_base_in, TEST_TENSOR_TYPE* const ext_base_out, const int width,
                  const int height, const int tile_width, const int tile_height) {
    TEST_TENSOR_TYPE(*const input_buffer)[height][width] = (TEST_TENSOR_TYPE(*)[height][width])ext_base_in;
    TEST_TENSOR_TYPE(*const output_buffer)[height][width] = (TEST_TENSOR_TYPE(*)[height][width])ext_base_out;
    bool result = true;

    for (int y = 0; y < height; y+=EVERY_N_LINES) {
        for (int x = 0; x < width; x++) {
            TEST_TENSOR_TYPE expected = input_buffer[0][y][x];

            if (true) {
                if (x > 0) expected += input_buffer[0][y][x - 1];
                if (y >= EVERY_N_LINES) expected += input_buffer[0][y - EVERY_N_LINES][x];
                if (x < (width - 1)) expected += input_buffer[0][y][x + 1];
                if (y < (height - EVERY_N_LINES)) expected += input_buffer[0][y + EVERY_N_LINES][x];
            }

            if (output_buffer[0][y][x] != expected) {
                printf("Mismatch at [%d, %d] " TEST_TENSOR_TYPE_SPECIFIER " != " TEST_TENSOR_TYPE_SPECIFIER
                       "Output Tensor size [%d, %d], Tile size [%d, %d]\n",
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