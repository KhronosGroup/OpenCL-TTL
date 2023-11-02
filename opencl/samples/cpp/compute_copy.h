/*
 * compute_cross.h
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

#define TILE_OVERLAP_LEFT 0
#define TILE_OVERLAP_RIGHT 0
#define TILE_OVERLAP_TOP 0
#define TILE_OVERLAP_BOTTOM 0

/*
 * @brief Compute code based on a tensor
 *
 * Compute the copy of the input tensor placing the result into the output tensor
 *
 * @param tensor_in The input tensor
 * @param tensor_out The output tensor
 *
 * We split compute into compute and compute_copy to demonstrate use of TTL_[read|write]_tensor
 * with a simple tensor
 */
void compute_copy(__TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, , _t) tensor_in,
             __TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, , _t) tensor_out) {

    for (int y = 0; y < tensor_in.shape.height; ++y) {
        for (int x = 0; x < tensor_out.shape.width; ++x) {
            TTL_write_tensor(tensor_out, TTL_read_tensor(tensor_in, x, y), x, y);
        }
    }
}

/*
 * @brief Compute code based on a tensor
 *
 * Compute the copy of the input tensor placing the result into the output tensor
 *
 * @param tensor_in The input sub tensor
 * @param tensor_out The output sub tensor
 *
 * We split compute into compute and compute_copy to demonstrate use of TTL_[read|write]_tensor
*/

void compute(__TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, sub_, _t) tensor_in,
             __TTL_tensor_name(TTL_, , int_, TEST_TENSOR_TYPE, sub_, _t) tensor_out) {
    compute_copy(tensor_in.tensor, tensor_out.tensor);
}