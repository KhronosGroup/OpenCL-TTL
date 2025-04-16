/*
 * ttl_duplex_buffering.cpp
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

#include "TTL/TTL.h"
#include "compute_cross.h"
#include "kernel.h"

/**
 * @brief Scope globally because it makes debugging easier
 */
static TEST_TENSOR_TYPE l_in[1024 * 512];
static TEST_TENSOR_TYPE l_out[1024 * 512];

bool TTL_duplex_buffering_kernel(TEST_TENSOR_TYPE *restrict ext_base_in, int external_stride_in,
                                 TEST_TENSOR_TYPE *restrict ext_base_out, int external_stride_out, TTL_dim width,
                                 TTL_dim height, TTL_dim tile_width, TTL_dim tile_height) {
    // Logical input tiling.
    const TTL_shape tensor_shape_in(width, height);
    const TTL_shape tile_shape_in(tile_width + (TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT),
                                  tile_height + (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM));
    const TTL_overlap overlap_in(TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT, TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM);
    const TTL_augmentation augmentation_in(
        TILE_OVERLAP_LEFT, TILE_OVERLAP_RIGHT, TILE_OVERLAP_TOP, TILE_OVERLAP_BOTTOM);
    const TTL_tiler input_tiler(tensor_shape_in, tile_shape_in, overlap_in, augmentation_in);

    // Logical output tiling.
    const TTL_shape tensor_shape_out(width, height);
    const TTL_tiler output_tiler(tensor_shape_out, TTL_shape(tile_width, tile_height));

    // External layouts.
    const TTL_layout ext_layout_in(external_stride_in);
    const TTL_layout ext_layout_out(external_stride_out);

    const TTL_tensor ext_input_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_tensor ext_output_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    // duplex_scheme must be defined outside, before the loop - because we "time-shift" the export to work on a recorded
    // tile written to in previous iteration.
    TTL_event sb_e_in_out[2] = { TTL_get_event(), TTL_get_event() };

    TTL_duplex_buffering duplex_scheme(
        ext_input_tensor, l_in, ext_output_tensor, l_out, &sb_e_in_out, input_tiler.get_tile(0));

    for (int i = 0; i < input_tiler.number_of_tiles(); ++i) {
        TTL_tile tile_next_import = input_tiler.get_tile(i);
        TTL_tile tile_current_export = output_tiler.get_tile(i);

        // Import current tile, export previous tile and wait for both transactions.
        TTL_io_tensors tensors = duplex_scheme.step_buffering(tile_next_import, tile_current_export);

        compute(ComputeType::TEST_COMPUTE_TYPE, tensors.imported_to, tensors.to_export_from);
    }

    duplex_scheme.finish_buffering();

    return result_check(ComputeType::TEST_COMPUTE_TYPE, ext_base_in, ext_base_out, width, height, tile_width, tile_height);
}
