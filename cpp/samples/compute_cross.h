/*
 * compute_cross.h
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

#include <assert.h>
#include <stdbool.h>

#include "TTL/TTL.h"

enum class ComputeType { CROSS, COPY };

constexpr TTL_dim TILE_OVERLAP_LEFT = ComputeType::TEST_COMPUTE_TYPE == ComputeType::CROSS ? 1 : 0;
constexpr TTL_dim TILE_OVERLAP_RIGHT = ComputeType::TEST_COMPUTE_TYPE == ComputeType::CROSS ? 1 : 0;
constexpr TTL_dim TILE_OVERLAP_TOP = ComputeType::TEST_COMPUTE_TYPE == ComputeType::CROSS ? 1 : 0;
constexpr TTL_dim TILE_OVERLAP_BOTTOM = ComputeType::TEST_COMPUTE_TYPE == ComputeType::CROSS ? 1 : 0;

template <typename TENSORTYPE,typename SUBTENSORSHAPETYPE, typename ORIGINALTENSORSHAPETYPE, typename LAYOUTTYPE>
void compute(const ComputeType compute_type, TTL_sub_tensor<TENSORTYPE, SUBTENSORSHAPETYPE, ORIGINALTENSORSHAPETYPE, LAYOUTTYPE> tensor_in,
             TTL_sub_tensor<TENSORTYPE, SUBTENSORSHAPETYPE, ORIGINALTENSORSHAPETYPE, LAYOUTTYPE> tensor_out) {
    for (TTL_dim y = 0; y < tensor_out.tensor.shape.height; ++y) {
        for (TTL_dim x = 0; x < tensor_out.tensor.shape.width; ++x) {
            const int x_in = x + TILE_OVERLAP_LEFT;
            const int y_in = y + TILE_OVERLAP_TOP;

            switch (compute_type) {
                case ComputeType::CROSS: {
                    const TENSORTYPE left = tensor_in.read(x_in - 1, y_in);
                    const TENSORTYPE above = tensor_in.read(x_in, y_in - 1);
                    const TENSORTYPE centre = tensor_in.read(x_in, y_in);
                    const TENSORTYPE right = tensor_in.read(x_in + 1, y_in);
                    const TENSORTYPE bottom = tensor_in.read(x_in, y_in + 1);
                    tensor_out.write(left + above + centre + right + bottom, x, y);
                    break;
                }
                case ComputeType::COPY: {
                    const TENSORTYPE centre = tensor_in.read(x_in, y_in);
                    tensor_out.write(centre, x, y);
                    break;
                }
                default: {
                    assert(false);
                }
            }
        }
    }
}

template <typename TENSORTYPE>
bool result_check(const ComputeType compute_type, TENSORTYPE* const ext_base_in, const uint32_t external_stide_in, TENSORTYPE* const ext_base_out, const uint32_t external_stide_out, const int width, const int height,
                  const int tile_width, const int tile_height) {
#define input_buffer ((TENSORTYPE(*)[height][external_stide_in])ext_base_in)
#define output_buffer ((TENSORTYPE(*)[height][external_stide_out])ext_base_out)

    // TENSORTYPE(* input_buffer)[height][width] = (TENSORTYPE(*)[height][width])ext_base_in;
    // TENSORTYPE(* output_buffer)[height][width] = (TENSORTYPE(*)[height][width])ext_base_out;
    bool result = true;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            TENSORTYPE expected = input_buffer[0][y][x];

            switch (compute_type) {
                case ComputeType::CROSS: {
                    if (x > 0) expected += input_buffer[0][y][x - 1];
                    if (y > 0) expected += input_buffer[0][y - 1][x];
                    if (x < (width - 1)) expected += input_buffer[0][y][x + 1];
                    if (y < (height - 1)) expected += input_buffer[0][y + 1][x];
                    break;
                }
                case ComputeType::COPY: {
                    // No change to expected value
                    break;
                }
                default: {
                    assert(false);
                }
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
