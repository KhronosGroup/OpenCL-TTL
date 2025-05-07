/*
 * main.c
 *
 * Copyright (c) 2025 Mobileye
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define TENSOR_WIDTH 103
#define TENSOR_HEIGHT 27
#define TILE_WIDTH 10
#define TILE_HEIGHT 10
#define restrict

#include "TTL/TTL.h"
#include "kernel.h"
typedef unsigned int unsigned_int;

static TEST_TENSOR_TYPE input_buffer[TENSOR_HEIGHT][TENSOR_WIDTH];
static TEST_TENSOR_TYPE output_buffer[TENSOR_HEIGHT][TENSOR_WIDTH];

int main(void) {
    for (uint32_t y = 0; y < TENSOR_HEIGHT; y++) {
        for (uint32_t x = 0; x < TENSOR_WIDTH; x++) {
            input_buffer[y][x] = x;
            output_buffer[y][x] = 0;
        }
    }

    if (KERNEL_NAME(&input_buffer[0][0],
                    TENSOR_WIDTH,
                    &output_buffer[0][0],
                    TENSOR_WIDTH,
                    TENSOR_WIDTH,
                    TENSOR_HEIGHT,
                    TILE_WIDTH,
                    TILE_HEIGHT) == true) {
        printf("Compute checked and successful\n");
    }
}