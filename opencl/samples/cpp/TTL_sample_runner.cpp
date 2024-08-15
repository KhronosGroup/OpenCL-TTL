/**********************************************************************
Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the
following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/
// For clarity,error checking has been omitted.

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#define xstr(s) str(s)
#define str(s) #s
#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A##B

#define CHECKER_NAME_STR xstr(M_CONC(check_, COMPUTE))
#define COMPUTE_NAME_STR xstr(M_CONC(compute_, COMPUTE))

#include "TTL/TTL.h"
#include CHECKER_NAME_STR

#define xstr(s) str(s)
#define str(s) #s

#undef TTL_EXT_TENSOR_TYPE
#define TTL_EXT_TENSOR_TYPE __TTL_tensor_name(TTL_, , ext_, TEST_TENSOR_TYPE, , _t)

#define SUCCESS 0
#define FAILURE 1
#define RETURN_STATUS_ON_ERROR(x)                                            \
    {                                                                        \
        cl_int status = (x);                                                 \
        if (status != 0) {                                                   \
            std::cout << "Status: " << status << " at line " << __LINE__ << std::endl; \
            return status;                                                   \
        }                                                                    \
    }

#define CHECK_STATUS(x)                                                      \
    {                                                                        \
        cl_int status = (x);                                                 \
        if (status != 0) {                                                   \
            std::cout << "Status: " << status << " at line " << __LINE__ << std::endl; \
            return FAILURE;                                                  \
        }                                                                    \
    }

/* convert the kernel file into a string */
int convertToString(const char *filename, std::string &s) {
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if (f.is_open()) {
        size_t size;
        size_t file_size;
        f.seekg(0, std::fstream::end);
        size = file_size = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        char *const str = new char[size + 1];

        if (!str) {
            f.close();
            return 0;
        }

        f.read(str, file_size);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return 0;
    }
    std::cout << "Error: failed to open file\n:" << filename << std::endl;
    return FAILURE;
}

/**
 * @brief Query the platform_id and choose the first GPU device if has one.Otherwise use the CPU as device.
 *
 * @param platform_id Id of the platform_id to search
 * @param device_id Location to place the id of the first GPU or CPU found
 * @param local_memory_size The size of the local memory of the device.
 *
 * @return OpenCl error code
 */
cl_int GetFirstDevice(const cl_platform_id platform_id, cl_device_id *const device_id, cl_ulong*const local_memory_size) {
    cl_uint num_device_ids = 0;

    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_device_ids);

    if (num_device_ids == 0)  // no GPU available.
    {
        std::cout << "No GPU device available, using CPU as default device." << std::endl;
        RETURN_STATUS_ON_ERROR(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 0, nullptr, &num_device_ids));
    }

    cl_device_id *const device_ids = new cl_device_id[num_device_ids];
    RETURN_STATUS_ON_ERROR(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, num_device_ids, device_ids, nullptr));
    *device_id = device_ids[0];

    delete[] device_ids;

    RETURN_STATUS_ON_ERROR(clGetDeviceInfo(*device_id,CL_DEVICE_LOCAL_MEM_SIZE , sizeof(*local_memory_size),local_memory_size,nullptr));

    return 0;
}

int main(int argc, char *argv[]) {
    /* Step1: Getting platform_ids and choose an available one.*/
    cl_uint num_platforms;                 // the NO. of platform_ids
    cl_platform_id platform_id = nullptr;  // the chosen platform_id
    CHECK_STATUS(clGetPlatformIDs(0, nullptr, &num_platforms));

    /*For clarity, choose the first available platform_id. */
    if (num_platforms > 0) {
        cl_platform_id *const platform_ids = new cl_platform_id[num_platforms];
        CHECK_STATUS(clGetPlatformIDs(num_platforms, platform_ids, nullptr));
        platform_id = platform_ids[0];
        delete[] platform_ids;
    }

    /* Step 2:Query the platform_id and choose the first GPU device if has one.Otherwise use the CPU as device.*/
    cl_device_id device_id;
    cl_ulong  local_memory_size;
    CHECK_STATUS(GetFirstDevice(platform_id, &device_id, &local_memory_size));

    /* Step 3: Create context.*/
    cl_context context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, nullptr);

    /* Step 4: Creating command queue associate with the context.*/
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, nullptr, nullptr);

    /* Step 5: Create program object */
    std::string source_string;
    CHECK_STATUS(convertToString("TTL_sample_overlap.cl", source_string));
    const char *source = source_string.c_str();
    size_t source_size[] = { strlen(source) };
    cl_program program = clCreateProgramWithSource(context, 1, &source, source_size, nullptr);

    const char *const ttl_include_library = std::getenv("TTL_INCLUDE_PATH");
    char command_line_options[256];

    sprintf(command_line_options,
            "-I %s -D TTL_COPY_3D  -D CL_TARGET_OPENCL_VERSION=300 -D COMPUTE_NAME=%s -D TEST_TENSOR_TYPE=%s -D LOCAL_MEMORY_SIZE=%ld",
            (ttl_include_library != nullptr)?ttl_include_library:".",
            COMPUTE_NAME_STR, xstr(TEST_TENSOR_TYPE), local_memory_size);

    /* Step 6: Build program. */
    if (clBuildProgram(program, 1, &device_id, command_line_options, nullptr, nullptr) != 0) {
        size_t size;

        // main code
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);

        char *const build_log = new char[size + 1];

        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, build_log, nullptr);
        std::cout << std::endl << std::endl << "Buildlog:   " << build_log << std::endl << std::endl;
        delete[] build_log;
        exit(-1);
    }

    cl_int error_code_ret;

    /* Step 7: Initial input,output for the host and create memory objects for the kernel*/
    constexpr uint32_t tensor_width = 500;
    constexpr uint32_t tensor_height = 500;
	constexpr uint32_t aligned_width = (tensor_width + (TTL_DEFAULT_EXTERNAL_ALIGNMENT - 1)) & ~(TTL_DEFAULT_EXTERNAL_ALIGNMENT - 1);
    TEST_TENSOR_TYPE (& input)[tensor_height][aligned_width] = *(TEST_TENSOR_TYPE (*)[tensor_height][aligned_width])new TEST_TENSOR_TYPE[tensor_height * aligned_width];
    TEST_TENSOR_TYPE (& output)[tensor_height][aligned_width] = *(TEST_TENSOR_TYPE (*)[tensor_height][aligned_width])new TEST_TENSOR_TYPE[tensor_height * aligned_width];

    for (int y = 0; y < tensor_height; y++) {
   		for (int x = 0; x < tensor_width ; x++) {
	        input[y][x] = rand();
    	    output[y][x] = rand();
		}
    }

    constexpr uint32_t cout_len = 2048;
    unsigned char *const cout_buffer = new unsigned char[cout_len];

    cl_mem std_out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, cout_len, (void *)nullptr, nullptr);

    const TTL_shape_t tensor_shape_in = TTL_create_shape(tensor_width, tensor_height);
    const TTL_shape_t tensor_shape_out = TTL_create_shape(tensor_width, tensor_height);
    const TTL_layout_t ext_layout_in = TTL_create_ext_layout(tensor_width);
    const TTL_layout_t ext_layout_out = TTL_create_ext_layout(tensor_width);
    const TTL_EXT_TENSOR_TYPE ext_input_tensor = TTL_create_ext_tensor(&input[0][0], tensor_shape_in, ext_layout_in);
    const TTL_EXT_TENSOR_TYPE ext_output_tensor = TTL_create_ext_tensor(&output[0][0], tensor_shape_out, ext_layout_out);
    void *const v = nullptr;

    /* Step 8: Create kernel object */
    cl_kernel kernel = clCreateKernel(program, "TTL_sample_overlap", nullptr);

    /* Step 9: Sets Kernel arguments.*/
    int arg = 0;
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(ext_input_tensor.base), (void *)&v));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(ext_input_tensor), (void *)&ext_input_tensor));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(ext_output_tensor), (void *)&ext_output_tensor));

    /* Step 10: Running the kernel.*/
    size_t global_work_size[1] = { 1 };
    cl_event event;
    CHECK_STATUS(
        clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, global_work_size, nullptr, 0, nullptr, &event));
    CHECK_STATUS(clWaitForEvents(1, &event));

    /* Step 11: Read the std and output and  back to host memory.*/
    CHECK_STATUS(clEnqueueReadBuffer(
        command_queue, std_out_buffer, CL_TRUE, 0, cout_len * sizeof(char), cout_buffer, 0, nullptr, nullptr));

    std::cout << (result_check(input, output, tensor_width, tensor_height) ? "Passed!" : "Failed") << std::endl;

    /* Step 13: Clean the resources.*/
    CHECK_STATUS(clReleaseKernel(kernel));    // Release kernel.
    CHECK_STATUS(clReleaseProgram(program));  // Release the program object.
    CHECK_STATUS(clReleaseMemObject(std_out_buffer));
    CHECK_STATUS(clReleaseCommandQueue(command_queue));  // Release  Command queue.
    CHECK_STATUS(clReleaseContext(context));             // Release context.

	delete[] &input;
	delete[] &output;

    return SUCCESS;
}
