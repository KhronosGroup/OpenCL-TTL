#!/usr/bin/python3

# ttl_sample_runner.py
#
# Copyright (c) 2023 Mobileye
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import ctypes
import pathlib

import numpy
import os
import sys
import random

def Read(byte_array, i, j, tensor_width, element_size):
    result = 0

    for byte_index in range(0, element_size):
        result = result + (pow(256, byte_index) * byte_array[(((i * tensor_width) + j) * element_size) + byte_index])
    
    return result

def TestTTL(program_name):
    os.environ["PYOPENCL_COMPILER_OUTPUT"] = "1"
    os.environ["PYOPENCL_CTX"] = "0"
    os.environ["PYOPENCL_NO_CACHE"] = "1"

    # Allow an environment variable to provide the TTL_INCLUDE_PATH, if not defined regular paths used.
    if "TTL_INCLUDE_PATH" in os.environ:
        ttl_include_path = "-I" + os.environ["TTL_INCLUDE_PATH"]
    else:
        ttl_include_path = ""

    # Allow an environment variable to provide the TTL_INCLUDE_PATH, if not defined regular paths used.
    if "TTL_EXTRA_DEFINES" in os.environ:
        ttl_extra_defines =  " " + os.environ["TTL_EXTRA_DEFINES"] + " "
    else:
        ttl_extra_defines = ""

    # For convenience remove any extension if it included.
    program_name = os.path.splitext(os.path.basename(program_name))[0]

    # For variation a number of tensor random sizes are used, then tiled with random tile sizes
    for test_tensor_type, test_tensor_size in list([('char', 1), ('uchar', 1), ('short', 2), ('ushort', 2), ('int',4), ('uint',4), ('long',8), ('ulong',8)]):
        program_name_type = program_name + "_" + test_tensor_type + ".so"
        compile_string = (
            "rm -f "
            + program_name_type
            + "; clang "
            + ttl_include_path
            + " -DTEST_TENSOR_TYPE="
            + test_tensor_type
            + " -DKERNEL_NAME="
            + program_name
            + " -DTTL_TARGET=c -fPIC -shared -o "
            + program_name_type
            + " "
            + program_name
            + ".c")
        os.system(compile_string)
        
        print("Testing %s with %s Tensors" % (program_name, test_tensor_type))

        for tensor_width in random.sample(range(1, 125), 5):
            for tensor_height in random.sample(range(1, 125), 5):
                for tile_width in [1, tensor_width] + random.sample(range(1, tensor_width + 30), 5):
                    for tile_height in [1, tensor_height] + random.sample(range(1, tensor_height + 30), 5):
                        error = False

                        output_data = bytearray(os.urandom(tensor_width * tensor_height * test_tensor_size))
                        input_data = bytearray(os.urandom(tensor_width * tensor_height * test_tensor_size))

                        input_buffer = ctypes.create_string_buffer(
                            bytes(input_data), len(input_data)
                        )
                        output_buffer = ctypes.create_string_buffer(
                            bytes(output_data), len(output_data)
                        )

                        c_lib = ctypes.CDLL(pathlib.Path().absolute() / (program_name_type))

                        getattr(c_lib, program_name)(
                            input_buffer,
                            tensor_width,
                            output_buffer,
                            tensor_width,
                            tensor_width,
                            tensor_height,
                            tile_width,
                            tile_height,
                        )

                        os.system("rm -f " + program_name_type)

                        return_buffer = bytearray(output_buffer.raw)

                        for i in range(0, tensor_height):
                            for j in range(0, tensor_width):
                                expected = Read(input_data, i, j, tensor_width, test_tensor_size)

                                if True:
                                    if j > 0:
                                        expected += Read(input_data, i, j - 1, tensor_width, test_tensor_size)
                                    if i  > 0:
                                        expected += Read(input_data, i - 1, j, tensor_width, test_tensor_size)
                                    if j < (tensor_width - 1):
                                        expected += Read(input_data, i, j + 1, tensor_width, test_tensor_size)
                                    if i < (tensor_height - 1):
                                        expected += Read(input_data, i + 1, j, tensor_width, test_tensor_size)
                                    
                                expected &= pow(256, test_tensor_size) - 1
                                actual = Read(return_buffer, i, j, tensor_width, test_tensor_size)

                                if actual != expected:
                                    print(
                                        "%s Failed at [%d, %d] %s != %s Tensor size [%d, %d], Tile size [%d, %d], Tensor type %s"
                                        % (
                                            program_name,
                                            j,
                                            i,
                                            hex(actual),
                                            hex(expected),
                                            tensor_width,
                                            tensor_height,
                                            tile_width,
                                            tile_height,
                                            test_tensor_type,
                                        )
                                    )

                                    error = True

                        if error:
                            exit(-1)

                    print("%s Passed Tensor size [%d, %d] Tile size [%d, %d] Type %s" %(program_name, tensor_width, tensor_height, tile_width, tile_height, test_tensor_type))


if __name__ == "__main__":
    for program_name in sys.argv[1:]:
        TestTTL(program_name)
