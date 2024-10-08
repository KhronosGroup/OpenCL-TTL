/*
 * kernel.h
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

bool KERNEL_NAME(TEST_TENSOR_TYPE *restrict ext_base_in, int external_stride_in,
                 TEST_TENSOR_TYPE *restrict ext_base_out, int external_stride_out, int width, int height,
                 int tile_width, int tile_height);
