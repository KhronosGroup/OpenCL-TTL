/*
 * ttl_duplex_buffering.cl
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

#undef TTL_IO_TENSORS_TYPE
#define TTL_IO_TENSORS_TYPE __TTL_tensor_name(TTL_io_, , , TEST_TENSOR_TYPE, , _t)
#undef TTL_DUPLEX_BUFFERING_TYPE
#define TTL_DUPLEX_BUFFERING_TYPE __TTL_tensor_name(TTL_duplex_, const_, , TEST_TENSOR_TYPE, , _buffering_t)
#undef TTL_EXT_TENSOR_TYPE
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TEST_TENSOR_TYPE, , _t)

#define LOCAL_TILE_SIZE (LOCAL_MEMORY_SIZE / sizeof(TEST_TENSOR_TYPE) / 2)

__kernel void TTL_duplex_buffering(__global TEST_TENSOR_TYPE *restrict ext_base_in, int external_stride_in,
                                   __global TEST_TENSOR_TYPE *restrict ext_base_out, int external_stride_out, int width,
                                   int height, int tile_width, int tile_height) {
    __local TEST_TENSOR_TYPE l_in[LOCAL_TILE_SIZE];
    __local TEST_TENSOR_TYPE l_out[LOCAL_TILE_SIZE];

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
    const TTL_layout_t ext_layout_in = TTL_create_layout(external_stride_in);
    const TTL_layout_t ext_layout_out = TTL_create_layout(external_stride_out);

    const TTL_EXT_TENSOR_TYPE ext_input_tensor = TTL_create_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_EXT_TENSOR_TYPE ext_output_tensor = TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    TTL_event_t sb_e_in_out[2] = { TTL_get_event(), TTL_get_event() };

    // The first data data from host->device starts here.
    TTL_DUPLEX_BUFFERING_TYPE duplex_scheme = TTL_start_duplex_buffering(
        ext_input_tensor, l_in, ext_output_tensor, l_out, &sb_e_in_out, TTL_get_tile(0, input_tiler));

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i, input_tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, output_tiler);

        // This waits for the current transfers to complete, and begins the next
        TTL_IO_TENSORS_TYPE tensors = TTL_step_buffering(&duplex_scheme, tile_next_import, tile_current_export);

        // Compute whilst the transfers are taking place (on separate buffers)
        compute(tensors.imported_to, tensors.to_export_from);
    }

    TTL_finish_buffering(&duplex_scheme);
}
