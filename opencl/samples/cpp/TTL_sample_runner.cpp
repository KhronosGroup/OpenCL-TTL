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

#include <OpenCL/cl.h>
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

#define FROM_C
#include "TTL/TTL.h"
#include CHECKER_NAME_STR

#define xstr(s) str(s)
#define str(s) #s

#define SUCCESS 0
#define FAILURE 1
#define RETURN_STATUS_ON_ERROR(x)                                            \
    {                                                                        \
        cl_int status = (x);                                                 \
        if (status != 0) {                                                   \
            cout << "Status: " << status << " at line " << __LINE__ << endl; \
            return status;                                                   \
        }                                                                    \
    }

#define CHECK_STATUS(x)                                                      \
    {                                                                        \
        cl_int status = (x);                                                 \
        if (status != 0) {                                                   \
            cout << "Status: " << status << " at line " << __LINE__ << endl; \
            return FAILURE;                                                  \
        }                                                                    \
    }

using namespace std;

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
    cout << "Error: failed to open file\n:" << filename << endl;
    return FAILURE;
}

/**
 * @brief Query the platform_id and choose the first GPU device if has one.Otherwise use the CPU as device.
 *
 * @param platform_id Id of the platform_id to search
 * @param device_id Location to place the id of the first GPU or CPU found
 *
 * @return OpenCl error code
 */
cl_int GetFirstDevice(const cl_platform_id platform_id, cl_device_id *const device_id) {
    cl_uint num_device_ids = 0;

    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_device_ids);

    if (num_device_ids == 0)  // no GPU available.
    {
        cout << "No GPU device available." << endl;
        cout << "Choose CPU as default device." << endl;
        RETURN_STATUS_ON_ERROR(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 0, nullptr, &num_device_ids));
    }

    cl_device_id *const device_ids = new cl_device_id[num_device_ids];
    RETURN_STATUS_ON_ERROR(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, num_device_ids, device_ids, nullptr));
    *device_id = device_ids[0];

    delete[] device_ids;

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
    CHECK_STATUS(GetFirstDevice(platform_id, &device_id));

    /* Step 3: Create context.*/
    cl_context context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, nullptr);

    /* Step 4: Creating command queue associate with the context.*/
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device_id, nullptr, nullptr);

    /* Step 5: Create program object */
    string source_string;
    CHECK_STATUS(convertToString("TTL_sample_overlap.cl", source_string));
    const char *source = source_string.c_str();
    size_t source_size[] = { strlen(source) };
    cl_program program = clCreateProgramWithSource(context, 1, &source, source_size, nullptr);

    const char *const ttl_include_library = std::getenv("TTL_INCLUDE_PATH");
    char command_line_options[128];

    if (ttl_include_library != nullptr) {
        sprintf(command_line_options,
                "-I %s -D TTL_COPY_3D -cl-std=CL3.0 -D CL_TARGET_OPENCL_VERSION=300 -D COMPUTE_NAME=%s",
                ttl_include_library,
                COMPUTE_NAME_STR);
    } else {
        sprintf(command_line_options,
                "-D TTL_COPY_3D -cl-std=CL3.0 -D CL_TARGET_OPENCL_VERSION=300 -D COMPUTE_NAME=%s",
                COMPUTE_NAME_STR);
    }

    /* Step 6: Build program. */
    if (clBuildProgram(program, 1, &device_id, command_line_options, nullptr, nullptr) != 0) {
        size_t size;

        // main code
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);

        char *const build_log = new char[size + 1];

        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, size, build_log, nullptr);
        cout << endl << endl << "Buildlog:   " << build_log << endl << endl;
        delete[] build_log;
        exit(-1);
    }

    cl_int error_code_ret;

    /* Step 7: Initial input,output for the host and create memory objects for the kernel*/
    constexpr uint32_t tensor_width = 5;
    constexpr uint32_t tensor_height = 5;
    unsigned char *const input = new unsigned char[tensor_width * tensor_height];
    unsigned char *const output = new unsigned char[tensor_width * tensor_height];

    for (int x = 0; x < (tensor_width * tensor_height); x++) {
        input[x] = rand();
        output[x] = rand();
    }

    cl_mem input_buffer =
        clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, tensor_width * tensor_height, input, &error_code_ret);
    CHECK_STATUS(error_code_ret);
    cl_mem output_buffer =
        clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, tensor_width * tensor_height, output, &error_code_ret);
    CHECK_STATUS(error_code_ret);

    constexpr uint32_t cout_len = 2048;
    unsigned char *const cout_buffer = new unsigned char[cout_len];

    cl_mem std_out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, cout_len, (void *)nullptr, nullptr);

    const TTL_shape_t tensor_shape_in = TTL_create_shape(tensor_width, tensor_height);
    const TTL_shape_t tensor_shape_out = TTL_create_shape(tensor_width, tensor_height);
    const TTL_layout_t ext_layout_in = TTL_create_layout(tensor_width);
    const TTL_layout_t ext_layout_out = TTL_create_layout(tensor_width);

    /* Step 8: Create kernel object */
    cl_kernel kernel = clCreateKernel(program, "TTL_sample_overlap", nullptr);

    /* Step 9: Sets Kernel arguments.*/
    int arg = 0;
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(input_buffer), &input_buffer));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(tensor_shape_in), &tensor_shape_in));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(ext_layout_in), &ext_layout_in));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(output_buffer), &output_buffer));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(tensor_shape_out), &tensor_shape_out));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(ext_layout_out), &ext_layout_out));

    /* Step 10: Running the kernel.*/
    size_t global_work_size[1] = { 1 };
    cl_event event;
    CHECK_STATUS(
        clEnqueueNDRangeKernel(command_queue, kernel, 1, nullptr, global_work_size, nullptr, 0, nullptr, &event));
    CHECK_STATUS(clWaitForEvents(1, &event));

    /* Step 11: Read the std and output and  back to host memory.*/
    CHECK_STATUS(clEnqueueReadBuffer(
        command_queue, std_out_buffer, CL_TRUE, 0, cout_len * sizeof(char), cout_buffer, 0, nullptr, nullptr));
    CHECK_STATUS(clEnqueueReadBuffer(command_queue,
                                     output_buffer,
                                     CL_TRUE,
                                     0,
                                     tensor_width * tensor_height * sizeof(char),
                                     output,
                                     0,
                                     nullptr,
                                     nullptr));

    std::cout << (result_check(input, output, tensor_width, tensor_height) ? "Passed!" : "Failed") << endl;

    /* Step 13: Clean the resources.*/
    CHECK_STATUS(clReleaseKernel(kernel));    // Release kernel.
    CHECK_STATUS(clReleaseProgram(program));  // Release the program object.
    CHECK_STATUS(clReleaseMemObject(std_out_buffer));
    CHECK_STATUS(clReleaseCommandQueue(command_queue));  // Release  Command queue.
    CHECK_STATUS(clReleaseContext(context));             // Release context.

    if (input != nullptr) {
        delete[] input;
    }

    if (output != nullptr) {
        delete[] output;
    }

    return SUCCESS;
}
