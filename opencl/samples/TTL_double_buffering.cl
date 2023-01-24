/*
* ttl_double_buffering.cl
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
#include "compute_cross.h"

#define MEMSZ 0x8000

__kernel void TTL_double_buffering(__global uchar *restrict ext_base_in, int external_stride_in,
                                   __global uchar *restrict ext_base_out, int external_stride_out, int width,
                                   int height, int tile_width, int tile_height) {
    local uchar l_in1[MEMSZ];
    local uchar l_in2[MEMSZ];
    local uchar l_out1[MEMSZ];
    local uchar l_out2[MEMSZ];

    // Logical input tiling.
    const TTL_shape_t tensor_shape_in = TTL_create_shape(width, height);
    const TTL_shape_t tile_shape_in = TTL_create_shape(tile_width + (TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT),
                                                          tile_height + (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM));
    const TTL_overlap_t overlap_in =
        TTL_create_overlap(TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT, TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM);
    const TTL_augmentation_t augmentation_in =
        TTL_create_augmentation(TILE_OVERLAP_LEFT, TILE_OVERLAP_RIGHT, TILE_OVERLAP_TOP, TILE_OVERLAP_BOTTOM);
    const TTL_tiler_t input_tiler =
        TTL_create_overlap_tiler(tensor_shape_in, tile_shape_in, overlap_in, augmentation_in);

    // Logical output tiling.
    const TTL_shape_t tensor_shape_out = TTL_create_shape(width, height);
    const TTL_tiler_t output_tiler = TTL_create_tiler(tensor_shape_out, TTL_create_shape(tile_width, tile_height));

    // External layouts.
    const TTL_layout_t ext_layout_in = TTL_create_layout(external_stride_in);
    const TTL_layout_t ext_layout_out = TTL_create_layout(external_stride_out);

    const TTL_const_ext_tensor_t ext_input_tensor = TTL_create_const_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_ext_tensor_t ext_output_tensor = TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    // import_db and export_db need to be defined outside, before the loop, as
    // they record the event to wait on
    TTL_event_t import_DB_e = TTL_get_event();
    TTL_import_double_buffering_t import_db = TTL_start_import_double_buffering(
        l_in1, l_in2, ext_input_tensor, &import_DB_e, TTL_get_tile(0, input_tiler));

    TTL_event_t export_DB_e = TTL_get_event();
    TTL_export_double_buffering_t export_db =
        TTL_start_export_double_buffering(l_out1, l_out2, ext_output_tensor, &export_DB_e);

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t t_next = TTL_get_tile(i + 1, input_tiler);
        // Wait for import #i and issue import #i+1
        TTL_int_sub_tensor_t imported_to = TTL_step_buffering(&import_db, t_next);

        TTL_tile_t t_curr = TTL_get_tile(i, output_tiler);
        // Wait for export #i-2 and issue export #i-1
        TTL_int_sub_tensor_t exported_from = TTL_step_buffering(&export_db, t_curr);

        compute(imported_to, exported_from);
    }

    TTL_finish_import_double_buffering(&import_db);
    TTL_finish_export_double_buffering(&export_db);
}
