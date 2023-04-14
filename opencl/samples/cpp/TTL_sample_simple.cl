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
#include "compute_square.h"

#define TILE_SIZE 0x8000
#define TILE_WIDTH 10
#define TILE_HEIGHT 10

__kernel void TTL_sample_simple_duplex(const __global TTL_ext_tensor_t *ptr_ext_input_tensor,
                                const __global TTL_ext_tensor_t *ptr_ext_output_tensor) {
    __local TTL_ext_tensor_t ext_input_tensor, ext_output_tensor;
    __local uchar l_in[TILE_SIZE], l_out[TILE_SIZE];

    TTL_fetch_tensor(&ext_input_tensor, ptr_ext_input_tensor);
    TTL_fetch_tensor(&ext_output_tensor, ptr_ext_output_tensor);

    // Tiling depends only on logical shapes:
    const TTL_shape_t tile_shape = TTL_create_shape(TILE_WIDTH, TILE_HEIGHT);
    const TTL_tiler_t tiler = TTL_create_tiler(ext_input_tensor.shape, tile_shape);

    TTL_event_t events[] = { TTL_get_event(), TTL_get_event() };

    TTL_duplex_buffering_t duplex_scheme = TTL_start_duplex_buffering(
        ext_input_tensor, l_in, ext_output_tensor, l_out, &events, TTL_get_tile(0, tiler));

    for (int i = 0; i < TTL_number_of_tiles(tiler); ++i) {
        const TTL_tile_t tile = TTL_get_tile(i, tiler);
        TTL_io_tensors_t tensors = TTL_step_buffering(&duplex_scheme, tile, tile);
        compute(tensors.imported_to, tensors.to_export_from);
    }

    TTL_finish_buffering(&duplex_scheme);
}

__kernel void TTL_sample_simple_double(const __global TTL_ext_tensor_t *ptr_ext_input_tensor,
                                const __global TTL_ext_tensor_t *ptr_ext_output_tensor) {
    __local TTL_const_ext_tensor_t ext_input_tensor;
    __local TTL_ext_tensor_t ext_output_tensor;
    __local uchar l_in1[TILE_SIZE], l_in2[TILE_SIZE], l_out1[TILE_SIZE], l_out2[TILE_SIZE];

    TTL_fetch_tensor(&ext_input_tensor, ptr_ext_input_tensor);
    TTL_fetch_tensor(&ext_output_tensor, ptr_ext_output_tensor);

    // Tiling depends only on logical shapes:
    const TTL_shape_t tile_shape = TTL_create_shape(TILE_WIDTH, TILE_HEIGHT);
    const TTL_tiler_t tiler = TTL_create_tiler(ext_input_tensor.shape, tile_shape);

    TTL_event_t import_event = TTL_get_event();
    TTL_import_double_buffering_t import_db = TTL_start_import_double_buffering(
        l_in1, l_in2, ext_input_tensor, &import_event, TTL_get_tile(0, tiler));

    TTL_event_t export_event = TTL_get_event();
    TTL_export_double_buffering_t export_db =
        TTL_start_export_double_buffering(l_out1, l_out2, ext_output_tensor, &export_event);

    for (int i = 0; i < TTL_number_of_tiles(tiler); ++i) {
        TTL_tile_t t_next = TTL_get_tile(i + 1, tiler);
        TTL_int_sub_tensor_t imported_to = TTL_step_buffering(&import_db, t_next);

        TTL_tile_t t_current = TTL_get_tile(i, tiler);
        TTL_int_sub_tensor_t exported_from = TTL_step_buffering(&export_db, t_current);

        compute(imported_to, exported_from);
    }

    TTL_finish_buffering(&import_db);
    TTL_finish_buffering(&export_db);
}
