/*
 * ttl_double_buffering.c
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
#include "fixed_tensor_sizes.h"

/**
 * @brief Scope globally because it makes debugging easier
 */
static TEST_TENSOR_TYPE input_buffer_1[1024 * 512];
static TEST_TENSOR_TYPE input_buffer_2[1024 * 512];
static TEST_TENSOR_TYPE output_buffer_1[1024 * 512];
static TEST_TENSOR_TYPE output_buffer_2[1024 * 512];

bool TTL_double_buffering_kernel(TEST_TENSOR_TYPE *restrict ext_base_in, int external_stride_in,
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

    // import_db and export_db need to be defined outside, before the loop, as
    // they record the event to wait on
    TTL_event import_DB_e = TTL_get_event();
    TTL_import_double_buffering import_db(
        input_buffer_1, input_buffer_2, ext_input_tensor, &import_DB_e, input_tiler.get_tile(0));

    TTL_event export_DB_e = TTL_get_event();
    TTL_export_double_buffering export_db(output_buffer_1, output_buffer_2, ext_output_tensor, &export_DB_e, output_tiler);

    for (int i = 0; i < input_tiler.number_of_tiles(); ++i) {
        TTL_tile tile_next_import = input_tiler.get_tile(i + 1);
        TTL_tile tile_current_export = output_tiler.get_tile(i);

        TTL_sub_tensor imported_to = import_db.step_buffering(tile_next_import);
        TTL_sub_tensor exported_from = export_db.step_buffering(tile_current_export);

        compute(ComputeType::TEST_COMPUTE_TYPE,imported_to, exported_from);
    }

    import_db.finish_buffering();
    export_db.finish_buffering();

    return result_check(ComputeType::TEST_COMPUTE_TYPE,                         ext_base_in,
						EXTERNAL_STRIDE_IN,
                        ext_base_out,EXTERNAL_STRIDE_OUT,
width, height, tile_width, tile_height);
}
