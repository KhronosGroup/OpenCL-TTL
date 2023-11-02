//
// Copyright (c) 2022 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <random>
#include <vector>

#include "harness/compat.h"
#include "harness/conversions.h"
#include "procs.h"

#define STR1(x) #x
#define STR(x) STR1(x)

#define TILE_OVERLAP_LEFT 1
#define TILE_OVERLAP_RIGHT 1
#define TILE_OVERLAP_TOP 1
#define TILE_OVERLAP_BOTTOM 1

// clang-format off

static const char *computeFunction = R"(
void compute(TTL_int_uchar_sub_tensor_t tensor_in, TTL_int_uchar_sub_tensor_t tensor_out) {
    for (int y = 0; y < tensor_out.tensor.shape.height; ++y) {
        for (int x = 0; x < tensor_out.tensor.shape.width; ++x) {
            const int x_in = x + TILE_OVERLAP_LEFT;
            const int y_in = y + TILE_OVERLAP_TOP;
            const uchar left = TTL_read_tensor(tensor_in, x_in - 1, y_in);
            const uchar above = TTL_read_tensor(tensor_in, x_in, y_in - 1);
            const uchar centre = TTL_read_tensor(tensor_in, x_in, y_in);
            const uchar right = TTL_read_tensor(tensor_in, x_in + 1, y_in);
            const uchar bottom = TTL_read_tensor(tensor_in, x_in, y_in + 1);

            TTL_write_tensor(tensor_out, left + above + centre + right + bottom, x, y);
        }
    }
}
)";

static const char *ttlDoubleBufferingKernel = R"(
#define TTL_COPY_3D
#include "%s/TTL.h"

#define TILE_OVERLAP_LEFT % d
#define TILE_OVERLAP_RIGHT % d
#define TILE_OVERLAP_TOP % d
#define TILE_OVERLAP_BOTTOM % d

% s
#define MEMSZ 0x8000

        __kernel void
        TTL_double_buffering(__global uchar *restrict ext_base_in, int external_stride_in,
                             __global uchar *restrict ext_base_out, int external_stride_out, int width, int height,
                             int tile_width, int tile_height) {
    local uchar l_in1[MEMSZ];
    local uchar l_in2[MEMSZ];
    local uchar l_out1[MEMSZ];
    local uchar l_out2[MEMSZ];

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

    const TTL_const_ext_uchar_tensor_t ext_input_tensor =
        TTL_create_const_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_ext_uchar_tensor_t ext_output_tensor =
        TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    // import_db and export_db need to be defined outside, before the loop, as
    // they record the event to wait on
    TTL_event_t import_DB_e = TTL_get_event();
    TTL_import_double_const_uchar_tensor_buffering_t import_db =
        TTL_start_import_double_buffering(l_in1, l_in2, ext_input_tensor, &import_DB_e, TTL_get_tile(0, input_tiler));

    TTL_event_t export_DB_e = TTL_get_event();
    TTL_export_double_const_uchar_tensor_buffering_t export_db =
        TTL_start_export_double_buffering(l_out1, l_out2, ext_output_tensor, &export_DB_e);

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t t_next = TTL_get_tile(i + 1, input_tiler);
        // Wait for import #i and issue import #i+1
        TTL_int_uchar_sub_tensor_t imported_to = TTL_step_buffering(&import_db, t_next);

        TTL_tile_t t_curr = TTL_get_tile(i, output_tiler);
        // Wait for export #i-2 and issue export #i-1
        TTL_int_uchar_sub_tensor_t exported_from = TTL_step_buffering(&export_db, t_curr);

        compute(imported_to, exported_from);
    }

    TTL_finish_buffering(&import_db);
    TTL_finish_buffering(&export_db);
}
)";

static const char *ttlSimplexBufferingKernel = R"(
#define TTL_COPY_3D
#include "%s/TTL.h"

#define TILE_OVERLAP_LEFT % d
#define TILE_OVERLAP_RIGHT % d
#define TILE_OVERLAP_TOP % d
#define TILE_OVERLAP_BOTTOM % d

% s
#define MEMSZ 0x8000

        __kernel void
        TTL_simplex_buffering(__global uchar *restrict ext_base_in, int external_stride_in,
                              __global uchar *restrict ext_base_out, int external_stride_out, int width, int height,
                              int tile_width, int tile_height) {
    __local uchar l_buff1[MEMSZ];
    __local uchar l_buff2[MEMSZ];
    __local uchar l_buff3[MEMSZ];

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

    const TTL_ext_uchar_tensor_t ext_input_tensor = TTL_create_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_ext_uchar_tensor_t ext_output_tensor =
        TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    TTL_event_t tb_e_in = TTL_get_event();
    TTL_event_t tb_e_out = TTL_get_event();
    TTL_simplex_const_uchar_tensor_buffering_t simplex_scheme =
        TTL_start_simplex_buffering(l_buff1,
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

        TTL_io_uchar_tensor_t tensors = TTL_step_buffering(&simplex_scheme, tile_next_import, tile_current_export);

        compute(tensors.imported_to, tensors.to_export_from);
    }

    TTL_finish_buffering(&simplex_scheme);
}
)";

static const char *ttlDuplexBufferingKernel = R"(
#define TTL_COPY_3D
#include "%s/TTL.h"

#define TILE_OVERLAP_LEFT % d
#define TILE_OVERLAP_RIGHT % d
#define TILE_OVERLAP_TOP % d
#define TILE_OVERLAP_BOTTOM % d

% s
#define MEMSZ 0x8000

        __kernel void
        TTL_duplex_buffering(__global uchar *restrict ext_base_in, int external_stride_in,
                             __global uchar *restrict ext_base_out, int external_stride_out, int width, int height,
                             int tile_width, int tile_height) {
    __local uchar l_in[MEMSZ];
    __local uchar l_out[MEMSZ];

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

    const TTL_ext_uchar_tensor_t ext_input_tensor = TTL_create_ext_tensor(ext_base_in, tensor_shape_in, ext_layout_in);
    const TTL_ext_uchar_tensor_t ext_output_tensor =
        TTL_create_ext_tensor(ext_base_out, tensor_shape_out, ext_layout_out);

    TTL_event_t sb_e_in_out[2] = { TTL_get_event(), TTL_get_event() };

    TTL_duplex_const_uchar_tensor_buffering_t duplex_scheme = TTL_start_duplex_buffering(
        ext_input_tensor, l_in, ext_output_tensor, l_out, &sb_e_in_out, TTL_get_tile(0, input_tiler));

    for (int i = 0; i < TTL_number_of_tiles(input_tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i, input_tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, output_tiler);

        // Import current tile, export previous tile and wait for both transactions.
        TTL_io_uchar_tensor_t tensors = TTL_step_buffering(&duplex_scheme, tile_next_import, tile_current_export);

        compute(tensors.imported_to, tensors.to_export_from);
    }

    TTL_finish_buffering(&duplex_scheme);
}
)";
// clang-format on

bool resultCheck(unsigned char *const extBaseIn, unsigned char *const extBaseOut, const int tensorWidth,
                 const int tensorHeight, const int tileWidth, const int tileHeight) {
    uint8_t(*const inputBuffer)[tensorHeight][tensorWidth] = (uint8_t(*)[tensorHeight][tensorWidth])extBaseIn;
    uint8_t(*const outputBuffer)[tensorHeight][tensorWidth] = (uint8_t(*)[tensorHeight][tensorWidth])extBaseOut;
    bool success = true;

    for (int y = 0; y < tensorHeight; y++) {
        for (int x = 0; x < tensorWidth; x++) {
            uint8_t expected = 0;

            if (x > 0) expected += inputBuffer[0][y][x - 1];
            if (y > 0) expected += inputBuffer[0][y - 1][x];

            expected += inputBuffer[0][y][x];

            if (x < (tensorWidth - 1)) expected += inputBuffer[0][y][x + 1];
            if (y < (tensorHeight - 1)) expected += inputBuffer[0][y + 1][x];

            if ((outputBuffer[0][y][x] != expected) && (success == true)) {
                log_info("test_ttl failed at [%d, %d] %d != %d "
                         "Tensor size "
                         "[%d, %d], Tile size [%d, %d]\n",
                         y,
                         x,
                         outputBuffer[0][y][x],
                         expected,
                         tensorWidth,
                         tensorHeight,
                         tileWidth,
                         tileHeight);
                success = false;
            }
        }
    }

    return success;
}

class random_list : public std::vector<uint32_t> {
public:
    random_list(const uint32_t lowest, uint32_t const highest, const uint32_t count, std::vector<uint32_t> start_vector)
        : std::vector<uint32_t>(start_vector) {
        std::random_device rd;                                   // obtain a random number from hardware
        std::mt19937 gen(rd());                                  // seed the generator
        std::uniform_int_distribution<> distr(lowest, highest);  // define the range

        for (int n = 0; n < count; ++n) {
            this->push_back(distr(gen));
        }
    }
};

static int test_ttl_all_types(cl_device_id deviceID, cl_context context, cl_command_queue queue,
                              const char *const kernelCode, const char *computeFunction, const char *const kernelName) {
    int error;
    clProgramWrapper program;
    clKernelWrapper kernel;
    size_t threads[1], localThreads[1];
    int elementSize = 8;
    bool failuresPrinted = false;

    log_info("Testing %s\n", kernelName);

    cl_long max_local_mem_size;
    error = clGetDeviceInfo(deviceID, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(max_local_mem_size), &max_local_mem_size, NULL);
    test_error(error, "clGetDeviceInfo for CL_DEVICE_LOCAL_MEM_SIZE failed.");

    unsigned int num_of_compute_devices;
    error = clGetDeviceInfo(
        deviceID, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(num_of_compute_devices), &num_of_compute_devices, NULL);
    test_error(error, "clGetDeviceInfo for CL_DEVICE_MAX_COMPUTE_UNITS failed.");

    char programSource[10240] = { 0 };
    char *programPtr;

    sprintf(programSource,
            kernelCode,
            STR(TTL_INSTALL_DIR),
            TILE_OVERLAP_LEFT,
            TILE_OVERLAP_RIGHT,
            TILE_OVERLAP_TOP,
            TILE_OVERLAP_BOTTOM,
            computeFunction);

    //log_info("program: %s\n", programSource);
    programPtr = programSource;

    error = create_single_kernel_helper(context, &program, &kernel, 1, (const char **)&programPtr, kernelName);
    test_error(error, "Unable to create testing kernel");

    size_t max_workgroup_size;
    error = clGetKernelWorkGroupInfo(
        kernel, deviceID, CL_KERNEL_WORK_GROUP_SIZE, sizeof(max_workgroup_size), &max_workgroup_size, NULL);
    test_error(error, "clGetKernelWorkGroupInfo failed for CL_KERNEL_WORK_GROUP_SIZE.");

    size_t max_local_workgroup_size[3];
    error = clGetDeviceInfo(
        deviceID, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_local_workgroup_size), max_local_workgroup_size, NULL);
    test_error(error, "clGetDeviceInfo failed for CL_DEVICE_MAX_WORK_ITEM_SIZES");

    // Pick the minimum of the device and the kernel
    if (max_workgroup_size > max_local_workgroup_size[0]) max_workgroup_size = max_local_workgroup_size[0];

    size_t numberOfCopiesPerWorkitem = 13;
    size_t localStorageSpacePerWorkitem = numberOfCopiesPerWorkitem * elementSize;
    size_t maxLocalWorkgroupSize = (((int)max_local_mem_size / 2) / localStorageSpacePerWorkitem);

    // Calculation can return 0 on embedded devices due to 1KB local mem limit
    if (maxLocalWorkgroupSize == 0) {
        maxLocalWorkgroupSize = 1;
    }

    size_t localWorkgroupSize = maxLocalWorkgroupSize;
    if (maxLocalWorkgroupSize > max_workgroup_size) localWorkgroupSize = max_workgroup_size;

    // For variation a number of tensor random sizes are used, then tiled with
    // random tile sizes
    for (uint32_t tensorWidth : random_list(1, 125, 20, {})) {
        for (uint32_t tensorHeight : random_list(1, 125, 20, {})) {
            size_t numberOfLocalWorkgroups = 1;  // 111;
            size_t globalBufferSize = tensorWidth * tensorHeight;
            size_t globalWorkgroupSize = numberOfLocalWorkgroups * localWorkgroupSize;

            uint8_t *const inBuffer = new uint8_t[globalBufferSize];
            uint8_t *const outBuffer = new uint8_t[globalBufferSize];
            memset(outBuffer, 0, globalBufferSize);

            threads[0] = 1;       // globalWorkgroupSize;
            localThreads[0] = 1;  // localWorkgroupSize;

            {
                const MTdata d = init_genrand(gRandomSeed);
                generate_random_data(kUChar, globalBufferSize, d, inBuffer);
                free_mtdata(d);
            }

            clMemWrapper input_stream =
                clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, globalBufferSize, inBuffer, &error);
            test_error(error, "Unable to create input buffer");
            clMemWrapper output_stream =
                clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, globalBufferSize, outBuffer, &error);
            test_error(error, "Unable to create output buffer");

            for (uint32_t tileWidth : random_list(1, tensorWidth + 30, 20, { 1, tensorWidth })) {
                for (uint32_t tileHeight : random_list(1, tensorHeight + 30, 20, { 1, tensorHeight })) {
                    error = clSetKernelArg(kernel, 0, sizeof(input_stream), &input_stream);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 1, sizeof(tensorWidth), &tensorWidth);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 2, sizeof(output_stream), &output_stream);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 3, sizeof(tensorWidth), &tensorWidth);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 4, sizeof(tensorWidth), &tensorWidth);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 5, sizeof(tensorHeight), &tensorHeight);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 6, sizeof(tileWidth), &tileWidth);
                    test_error(error, "Unable to set kernel argument");
                    error = clSetKernelArg(kernel, 7, sizeof(tileHeight), &tileHeight);
                    test_error(error, "Unable to set kernel argument");

                    cl_event completion_event;

                    // Enqueue
                    error = clEnqueueNDRangeKernel(
                        queue, kernel, 1, NULL, threads, localThreads, 0, NULL, &completion_event);
                    test_error(error, "Unable to queue kernel");
                    error = clWaitForEvents(1, &completion_event);
                    test_error(error, "Unable to wait for kernel");

                    // Read
                    cl_event read_buffer_event;
                    error = clEnqueueReadBuffer(
                        queue, output_stream, CL_TRUE, 0, globalBufferSize, outBuffer, 0, NULL, &read_buffer_event);
                    test_error(error, "Unable to read results");
                    error = clWaitForEvents(1, &read_buffer_event);
                    test_error(error, "Unable to wait for read buffer");

                    bool failed = false;

                    if ((failed == false) &&
                        (resultCheck(inBuffer, outBuffer, tensorWidth, tensorHeight, tileWidth, tileHeight) == false)) {
                        failed = true;
                        failuresPrinted = true;
                    }
                }
            }
        }
    }

    return failuresPrinted ? -1 : 0;
}

int test_ttl(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements) {
    return ((test_ttl_all_types(
                 deviceID, context, queue, ttlDoubleBufferingKernel, computeFunction, "TTL_double_buffering") == 0) &&
            (test_ttl_all_types(
                 deviceID, context, queue, ttlSimplexBufferingKernel, computeFunction, "TTL_simplex_buffering") == 0) &&
            (test_ttl_all_types(
                 deviceID, context, queue, ttlDuplexBufferingKernel, computeFunction, "TTL_duplex_buffering") == 0))
               ? 0
               : -1;
}
