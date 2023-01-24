/*
 * ttl_double_buffering.c
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

/**
 * @brief Scope globally because it makes debugging easier
 */
static unsigned char input_buffer_1[1024 * 512];
static unsigned char input_buffer_2[1024 * 512];
static unsigned char output_buffer_1[1024 * 512];
static unsigned char output_buffer_2[1024 * 512];

void TTL_double_buffering(unsigned char *restrict ext_base_in, int external_stride_in,
                          unsigned char *restrict ext_base_out, int external_stride_out, int width, int height,
                          int tile_width, int tile_height) {
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

    const TTL_const_ext_tensor_t ext_input_tensor =
        TTL_create_const_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_ext_tensor_t ext_output_tensor = TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    // import_db and export_db need to be defined outside, before the loop, as
    // they record the event to wait on
    TTL_event_t import_DB_e = TTL_get_event();
    TTL_import_double_buffering_t import_db = TTL_start_import_double_buffering(
        input_buffer_1, input_buffer_2, ext_input_tensor, &import_DB_e, TTL_get_tile(0, input_tiler));

    TTL_event_t export_DB_e = TTL_get_event();
    TTL_export_double_buffering_t export_db =
        TTL_start_export_double_buffering(output_buffer_1, output_buffer_2, ext_output_tensor, &export_DB_e);

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i + 1, input_tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, output_tiler);

        TTL_int_sub_tensor_t imported_to = TTL_step_buffering(&import_db, tile_next_import);
        TTL_int_sub_tensor_t exported_from = TTL_step_buffering(&export_db, tile_current_export);

        compute(imported_to, exported_from);
    }

    TTL_finish_import_double_buffering(&import_db);
    TTL_finish_export_double_buffering(&export_db);

    result_check(ext_base_in, ext_base_out, width, height, tile_width, tile_height);
}
