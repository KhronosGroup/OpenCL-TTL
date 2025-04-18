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

#undef TTL_INT_SUB_TENSOR_TYPE
#define TTL_INT_SUB_TENSOR_TYPE __TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, sub_, _t)
#undef TTL_IMPORT_DOUBLE_BUFFERING_TYPE
#define TTL_IMPORT_DOUBLE_BUFFERING_TYPE \
    __TTL_tensor_name(TTL_import_double_, const_, , TEST_TENSOR_TYPE, , _buffering_t)
#undef TTL_EXPORT_DOUBLE_BUFFERING_TYPE
#define TTL_EXPORT_DOUBLE_BUFFERING_TYPE \
    __TTL_tensor_name(TTL_export_double_, const_, , TEST_TENSOR_TYPE, , _buffering_t)
#undef TTL_EXT_TENSOR_TYPE
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TEST_TENSOR_TYPE, , _t)
#undef TTL_CONST_EXT_TENSOR_TYPE
#define TTL_CONST_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, const_, ext_, TEST_TENSOR_TYPE, , _t)

#define LOCAL_TILE_SIZE (LOCAL_MEMORY_SIZE / sizeof(TEST_TENSOR_TYPE) / 4)

__kernel void TTL_double_buffering(__global TEST_TENSOR_TYPE *restrict ext_base_in, int external_stride_in,
                                   __global TEST_TENSOR_TYPE *restrict ext_base_out, int external_stride_out, int width,
                                   int height, int tile_width, int tile_height) {
    local TEST_TENSOR_TYPE input_buffer_1[LOCAL_TILE_SIZE];
    local TEST_TENSOR_TYPE input_buffer_2[LOCAL_TILE_SIZE];
    local TEST_TENSOR_TYPE output_buffer_1[LOCAL_TILE_SIZE];
    local TEST_TENSOR_TYPE output_buffer_2[LOCAL_TILE_SIZE];

    if (((TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT + tile_width) *
         (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM + tile_height)) > LOCAL_TILE_SIZE) {
        printf("Tile too large %d > %lu\n",
               ((TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT + tile_width) *
                (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM + tile_height)),
               LOCAL_TILE_SIZE);
        return;
    }

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
    const TTL_layout_t ext_layout_in = TTL_create_ext_layout(external_stride_in);
    const TTL_layout_t ext_layout_out = TTL_create_ext_layout(external_stride_out);

    const TTL_CONST_EXT_TENSOR_TYPE ext_input_tensor =
        TTL_create_const_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_EXT_TENSOR_TYPE ext_output_tensor = TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    // import_db and export_db need to be defined outside, before the loop, as
    // they record the event to wait on
    TTL_event_t import_DB_e = TTL_get_event();
    TTL_IMPORT_DOUBLE_BUFFERING_TYPE import_db = TTL_start_import_double_buffering(
        input_buffer_1, input_buffer_2, ext_input_tensor, &import_DB_e, TTL_get_tile(0, input_tiler));

    TTL_event_t export_DB_e = TTL_get_event();
    TTL_EXPORT_DOUBLE_BUFFERING_TYPE export_db =
        TTL_start_export_double_buffering(output_buffer_1, output_buffer_2, ext_output_tensor, &export_DB_e);

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i + 1, input_tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, output_tiler);

        TTL_INT_SUB_TENSOR_TYPE imported_to = TTL_step_buffering(&import_db, tile_next_import);
        TTL_INT_SUB_TENSOR_TYPE exported_from = TTL_step_buffering(&export_db, tile_current_export);

        compute(imported_to, exported_from);
    }

    TTL_finish_buffering(&import_db);
    TTL_finish_buffering(&export_db);
}
