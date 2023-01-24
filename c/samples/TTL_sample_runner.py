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

def TestTTL(program_name):
    os.environ['PYOPENCL_COMPILER_OUTPUT'] = '1'
    os.environ['PYOPENCL_CTX'] = '0'
    os.environ["PYOPENCL_NO_CACHE"] = "1"
    
    # Allow an environment variable to provide the TTL_INCLUDE_PATH, if not defined regular paths used.
    if "TTL_INCLUDE_PATH" in os.environ:
        ttl_include_path = "-I" + os.environ["TTL_INCLUDE_PATH"]
    else:
        ttl_include_path = ""

    # For convenience remove any extension if it included.
    program_name = os.path.splitext(os.path.basename(program_name))[0]

    compile_string = "rm -f " + program_name +".so; clang " + ttl_include_path + " -DKERNEL_NAME=" + program_name +" -DTTL_TARGET=c -fPIC -shared -o " + program_name +".so " + program_name + ".c"
    os.system(compile_string)
    c_lib = ctypes.CDLL(pathlib.Path().absolute() / (program_name + ".so"))

    # For variation a number of tensor random sizes are used, then tiled with random tile sizes
    for tensor_width in random.sample(range(1, 125), 15):
        for tensor_height in random.sample(range(1, 125), 15):
            output_data = bytearray(os.urandom(tensor_width * tensor_height  + 0))
            input_data = bytearray(os.urandom(tensor_width * tensor_height  + 1))

            input_buffer = ctypes.create_string_buffer(bytes(input_data), len(input_data));
            output_buffer = ctypes.create_string_buffer(bytes(output_data), len(output_data));

            for tile_width in [1, tensor_width] + random.sample(range(1, tensor_width+30), 15):
                for tile_height in [1, tensor_height] + random.sample(range(1, tensor_height+30), 15):
                    error = False

                    getattr(c_lib, program_name)(input_buffer, tensor_width, output_buffer, tensor_width, tensor_width, tensor_height, tile_width, tile_height)

                    return_buffer = bytearray(output_buffer.raw)

                    for i in range(0, tensor_height):
                        for j in range(0, tensor_width):
                            expected = 0

                            if True:
                                if j > 0:
                                    expected += input_data[i * tensor_width + (j - 1)];
                                if i  > 0:
                                    expected += input_data[(i -1 )* tensor_width + j];

                                expected += input_data[i * tensor_width + j];

                                if j < (tensor_width - 1):
                                    expected += input_data[i * tensor_width + (j + 1)]
                                if i < (tensor_height - 1):
                                    expected += input_data[(i + 1) * tensor_width + j]
                            else:
                                expected = (input_data[i * tensor_width + j] + 1) * (input_data[i * tensor_width + j] + 2) * (input_data[i * tensor_width + j] + 3)
                                
                            expected &= 0xff

                            if return_buffer[i * tensor_width + j] != expected:
                                print("%s Failed at [%d, %d] %d != %d Tensor size [%d, %d], Tile size [%d, %d]" %(program_name, j, i, return_buffer[i * tensor_width + j], expected, tensor_width, tensor_height, tile_width, tile_height))
                                error = True
                                exit(-1)
                    
                    if error:
                        exit(-1)

                    print("%s Passed Tensor size [%d, %d] Tile size [%d, %d]" %(program_name, tensor_width, tensor_height, tile_width, tile_height))

    os.system("rm -f " + program_name +".so")

if __name__ == '__main__':
    for program_name in sys.argv[1:]:
        TestTTL(program_name)
