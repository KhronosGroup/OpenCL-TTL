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

template <typename TestTensor>
bool result_check(const TestTensor&  ext_base_in, const TestTensor& ext_base_out, const int width,
                  const int height) {
    bool result = true;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            TEST_TENSOR_TYPE expected = ext_base_in[y][x];

            if (ext_base_out[y][x] != expected) {
                printf("Mismatch at [%d, %d] " TEST_TENSOR_TYPE_SPECIFIER " != " TEST_TENSOR_TYPE_SPECIFIER " Tensor size [%d, %d]\n",
                       x,
                       y,
                       ext_base_out[y][x],
                       expected,
                       width,
                       height);
                       result = false;
            }
        }
    }
#undef input_buffer
#undef output_buffer

    return result;
}