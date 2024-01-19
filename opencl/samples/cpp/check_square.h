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

bool result_check(unsigned char* const ext_base_in, unsigned char* const ext_base_out, const int width,
                  const int height) {
#define input_buffer ((uint8_t(*)[height][width])ext_base_in)
#define output_buffer ((uint8_t(*)[height][width])ext_base_out)
    bool result = true;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char expected = input_buffer[0][y][x] ^ 2;

            if (output_buffer[0][y][x] != expected) {
                printf("Mismatch at [%d, %d] %d != %d Tensor size [%d, %d]\n",
                       x,
                       y,
                       output_buffer[0][y][x],
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
