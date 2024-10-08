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

#define xstr(s) str(s)
#define str(s) #s
#define COMPUTE_NAME_1 xstr(COMPUTE_NAME)
#include COMPUTE_NAME_1

#define TILE_WIDTH 10
#define TILE_HEIGHT 10
#define LOCAL_TILE_SIZE (LOCAL_MEMORY_SIZE / sizeof(TEST_TENSOR_TYPE) / 2)

#undef TTL_IO_TENSORS_TYPE
#define TTL_IO_TENSORS_TYPE __TTL_tensor_name(TTL_io_, , , TEST_TENSOR_TYPE, , _t)
#undef TTL_DUPLEX_BUFFERING_TYPE
#define TTL_DUPLEX_BUFFERING_TYPE __TTL_tensor_name(TTL_duplex_, const_, , TEST_TENSOR_TYPE, , _buffering_t)
#undef TTL_EXT_TENSOR_TYPE
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TEST_TENSOR_TYPE, , _t)

__kernel void TTL_sample_overlap(__global TEST_TENSOR_TYPE *unused_ptr_1, const TTL_EXT_TENSOR_TYPE ext_input_tensor,
                                 const TTL_EXT_TENSOR_TYPE ext_output_tensor) {
    __local TEST_TENSOR_TYPE l_in[LOCAL_TILE_SIZE], l_out[LOCAL_TILE_SIZE];

    if (((TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT + TILE_WIDTH) *
         (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM + TILE_HEIGHT)) > LOCAL_TILE_SIZE) {
        printf("Tile too large %d > %lu\n",
               ((TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT + TILE_WIDTH) *
                (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM + TILE_HEIGHT)),
               LOCAL_TILE_SIZE);
        return;
    }
    // Logical input tiling.
    const TTL_shape_t tile_shape_in = TTL_create_shape(TILE_WIDTH + (TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT),
                                                       TILE_HEIGHT + (TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM));
    const TTL_overlap_t overlap_in =
        TTL_create_overlap(TILE_OVERLAP_LEFT + TILE_OVERLAP_RIGHT, TILE_OVERLAP_TOP + TILE_OVERLAP_BOTTOM);
    const TTL_augmentation_t augmentation_in =
        TTL_create_augmentation(TILE_OVERLAP_LEFT, TILE_OVERLAP_RIGHT, TILE_OVERLAP_TOP, TILE_OVERLAP_BOTTOM);
    const TTL_tiler_t input_tiler =
        TTL_create_overlap_tiler(ext_input_tensor.shape, tile_shape_in, overlap_in, augmentation_in);

    // Logical output tiling.
    const TTL_tiler_t output_tiler =
        TTL_create_tiler(ext_input_tensor.shape, TTL_create_shape(TILE_WIDTH, TILE_HEIGHT));

    // duplex_scheme must be defined outside, before the loop - because we "time-shift" the export to work on a recorded
    // tile written to in previous iteration.
    TTL_event_t sb_e_in_out[2] = { TTL_get_event(), TTL_get_event() };

    TTL_DUPLEX_BUFFERING_TYPE duplex_scheme = TTL_start_duplex_buffering(
        ext_input_tensor, l_in, ext_output_tensor, l_out, &sb_e_in_out, TTL_get_tile(0, input_tiler));

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i, input_tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, output_tiler);

        // Import current tile, export previous tile and wait for both transactions.
        TTL_IO_TENSORS_TYPE tensors = TTL_step_buffering(&duplex_scheme, tile_next_import, tile_current_export);

        compute(tensors.imported_to, tensors.to_export_from);
    }

    TTL_finish_buffering(&duplex_scheme);
}
