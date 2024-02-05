/*
 * main.c
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

#include <stdint.h>
#include <stdio.h>

#include "kernel.h"

#define TENSOR_WIDTH 100
#define INPUT_TENSOR_HEIGHT 200
#define OUTPUT_TENSOR_HEIGHT 100
#define TILE_WIDTH 10
#define TILE_HEIGHT 10

#include "TTL/TTL.h"

typedef unsigned int unsigned_int;

static TEST_TENSOR_TYPE input_buffer[INPUT_TENSOR_HEIGHT][TENSOR_WIDTH];
static TEST_TENSOR_TYPE output_buffer[OUTPUT_TENSOR_HEIGHT][TENSOR_WIDTH];

int main(void) {
    for (uint32_t x = 0; x < TENSOR_WIDTH; x++) {
        for (uint32_t y = 0; y < INPUT_TENSOR_HEIGHT; y++) {
            input_buffer[y][x] = x + y;
        }
        for (uint32_t y = 0; y < OUTPUT_TENSOR_HEIGHT; y++) {
            output_buffer[y][x] = 0;
        }
    }

    if (KERNEL_NAME(&input_buffer[0][0],
                    INPUT_TENSOR_HEIGHT,
                    TENSOR_WIDTH,
                    TENSOR_WIDTH,
                    &output_buffer[0][0],
                    OUTPUT_TENSOR_HEIGHT,
                    TENSOR_WIDTH,
                    TENSOR_WIDTH,
                    TILE_WIDTH,
                    TILE_HEIGHT) == true) {
        printf("Compute checked and successful\n");
    }
}