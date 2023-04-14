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

#include <fstream>
#include <iostream>
#include <string>

#define FROM_C
#include "TTL/TTL.h"
#include CHECKER_NAME

#define xstr(s) str(s)
#define str(s) #s

#define SUCCESS 0
#define FAILURE 1
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
        char *str;
        size_t file_size;
        f.seekg(0, std::fstream::end);
        size = file_size = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size + 1];
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

int main(int argc, char *argv[]) {
    /* Step1: Getting platforms and choose an available one.*/
    cl_uint numPlatforms;            // the NO. of platforms
    cl_platform_id platform = NULL;  // the chosen platform
    CHECK_STATUS(clGetPlatformIDs(0, NULL, &numPlatforms));

    /*For clarity, choose the first available platform. */
    if (numPlatforms > 0) {
        cl_platform_id *platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
        CHECK_STATUS(clGetPlatformIDs(numPlatforms, platforms, NULL));
        platform = platforms[0];
        free(platforms);
    }

    /* Step 2:Query the platform and choose the first GPU device if has one.Otherwise use the CPU as device.*/
    cl_uint num_devices = 0;
    cl_device_id *devices;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);

    if (num_devices == 0)  // no GPU available.
    {
        cout << "No GPU device available." << endl;
        cout << "Choose CPU as default device." << endl;
        CHECK_STATUS(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices));
        devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
        CHECK_STATUS(clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, num_devices, devices, NULL));
    } else {
        devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
        CHECK_STATUS(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL));
    }

    /* Step 3: Create context.*/
    cl_context context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

    /* Step 4: Creating command queue associate with the context.*/
    cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context, devices[0], NULL, NULL);

    /* Step 5: Create program object */
    string source_string;
    CHECK_STATUS(convertToString(xstr(KERNAL_SOURCE_NAME), source_string));
    const char *source = source_string.c_str();
    size_t source_size[] = { strlen(source) };
    cl_program program = clCreateProgramWithSource(context, 1, &source, source_size, NULL);

    /* Step 6: Build program. */
    if (clBuildProgram(
            program, 1, devices, "-I /localdisk/cgearing/ -D TTL_COPY_3D", NULL, NULL) !=
        0) {
        size_t size;

        // main code
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &size);

        char *build_log = new char[size + 1];

        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, size, build_log, NULL);
        cout << endl << endl << "Buildlog:   " << build_log << endl << endl;
        delete[] build_log;
        exit(-1);
    }

    /* Step 7: Initial input,output for the host and create memory objects for the kernel*/
    constexpr uint32_t input_width = 100;
    constexpr uint32_t input_height = 250;
    unsigned char *input = (unsigned char *)malloc(input_width * input_height);

    constexpr uint32_t output_width = 100;
    constexpr uint32_t output_height = 250;
    unsigned char *output = (unsigned char *)malloc(output_width * output_height);

    constexpr uint32_t cout_len = 2048;
    unsigned char *cout_buffer = (unsigned char *)malloc(cout_len * sizeof(unsigned char));

    for (int x = 0; x < sizeof(input_width * input_height); x++) {
        input[x] = rand();
        output[x] = rand();
    }

    cl_mem outputBuffer = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY , cout_len * sizeof(char), (void *)NULL, NULL);

    const TTL_shape_t tensor_shape_in = TTL_create_shape(input_width, input_height);
    const TTL_shape_t tensor_shape_out = TTL_create_shape(input_width, input_height);
    const TTL_layout_t ext_layout_in = TTL_create_layout(input_width);
    const TTL_layout_t ext_layout_out = TTL_create_layout(input_width);
    const TTL_ext_tensor_t ext_input_tensor = TTL_create_ext_tensor(input, tensor_shape_in, ext_layout_in) ;
    const TTL_ext_tensor_t ext_output_tensor = TTL_create_ext_tensor(output, tensor_shape_out, ext_layout_out);

    cl_int error_code_ret;
    cl_mem ext_input_tensor_mem = clCreateBuffer(context,
                                        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                        sizeof(ext_input_tensor),
                                        (void *)&ext_input_tensor,
                                        &error_code_ret);
    cl_mem ext_output_tensor_mem = clCreateBuffer(context,
                                        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                        sizeof(ext_output_tensor),
                                        (void *)&ext_output_tensor,
                                        &error_code_ret);
  
    /* Step 8: Create kernel object */
    cl_kernel kernel = clCreateKernel(program, xstr(KERNEL_NAME), NULL);

    /* Step 9: Sets Kernel arguments.*/
    int arg = 0;
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(cl_mem), (void *)&ext_input_tensor_mem));
    CHECK_STATUS(clSetKernelArg(kernel, arg++, sizeof(cl_mem), (void *)&ext_output_tensor_mem));

    /* Step 10: Running the kernel.*/
    size_t global_work_size[1] = { 1 };
    CHECK_STATUS(clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL));

    /* Step 11: Read the cout put back to host memory.*/
    CHECK_STATUS(clEnqueueReadBuffer(
        commandQueue, outputBuffer, CL_TRUE, 0, cout_len * sizeof(char), cout_buffer, 0, NULL, NULL));

    std::cout << (result_check(input, output, input_width, input_height)?"Passed!":"Failed") << endl;

    /* Step 12: Clean the resources.*/
    CHECK_STATUS(clReleaseKernel(kernel));          // Release kernel.
    CHECK_STATUS(clReleaseProgram(program));        // Release the program object.
    CHECK_STATUS(clReleaseMemObject(outputBuffer));
    CHECK_STATUS(clReleaseCommandQueue(commandQueue));  // Release  Command queue.
    CHECK_STATUS(clReleaseContext(context));            // Release context.

    if (input != NULL) {
        free(input);
        input = NULL;
    }

    if (output != NULL) {
        free(output);
        output = NULL;
    }

    if (devices != NULL) {
        free(devices);
        devices = NULL;
    }

    return SUCCESS;
}
