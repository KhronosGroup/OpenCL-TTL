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

void compute(TTL_int_sub_tensor_t tensor_in, TTL_int_sub_tensor_t tensor_out) {
    __local const unsigned char* const restrict l_in = (__local const unsigned char*)tensor_in.tensor.base;
    __local unsigned char* const restrict l_out = (__local unsigned char*)tensor_out.tensor.base;

    for (int y = 0; y < tensor_out.tensor.shape.height; ++y) {
        for (int x = 0; x < tensor_out.tensor.shape.width; ++x) {
            l_out[(y * tensor_in.tensor.layout.row_spacing) + (x)] =
                l_in[(y * tensor_out.tensor.layout.row_spacing) + (x)]^2;
        }
    }
}
