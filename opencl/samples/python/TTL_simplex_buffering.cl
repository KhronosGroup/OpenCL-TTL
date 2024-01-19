/*
* ttl_simplex_buffering.cl
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
#undef TTL_SIMPLEX_BUFFERING_TYPE
#define TTL_SIMPLEX_BUFFERING_TYPE __TTL_tensor_name(TTL_simplex_, const_, , TEST_TENSOR_TYPE, , _buffering_t)
#undef TTL_EXT_TENSOR_TYPE
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TEST_TENSOR_TYPE, , _t)

#define MEMSZ 0x8000

__kernel void TTL_simplex_buffering(__global TEST_TENSOR_TYPE *restrict ext_base_in, int external_stride_in,
                                    __global TEST_TENSOR_TYPE *restrict ext_base_out, int external_stride_out, int width,
                                    int height, int tile_width, int tile_height) {
    __local TEST_TENSOR_TYPE l_buff1[MEMSZ];
    __local TEST_TENSOR_TYPE l_buff2[MEMSZ];
    __local TEST_TENSOR_TYPE l_buff3[MEMSZ];

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

    TTL_event_t tb_e_in = TTL_get_event();
    TTL_event_t tb_e_out = TTL_get_event();
    TTL_SIMPLEX_BUFFERING_TYPE simplex_scheme = TTL_start_simplex_buffering(l_buff1,
                                                                    l_buff2,
                                                                    l_buff3,
                                                                    ext_input_tensor,
                                                                    ext_output_tensor,
                                                                    &tb_e_in,
                                                                    &tb_e_out,
                                                                    TTL_get_tile(0, input_tiler));

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i + 1, input_tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, output_tiler);

        TTL_IO_TENSORS_TYPE tensors = TTL_step_buffering(&simplex_scheme, tile_next_import, tile_current_export);

        compute(tensors.imported_to, tensors.to_export_from);
    }

    TTL_finish_buffering(&simplex_scheme);
}
