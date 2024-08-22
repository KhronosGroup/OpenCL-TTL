#!/usr/bin/python3

# ttl_sample_runner.py
#
# Copyright (c) 2025 Mobileye
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

def GetClangVerson(clang_command):
    import subprocess
    import re

    try:
        clang_version_output = subprocess.check_output([clang_command, "--version"]).decode("utf-8")
        match = re.search(r"version (\d+)\.(\d+)\.(\d+)", clang_version_output)
        if match:
            major, minor, patch = map(int, match.groups())
            return major
        else:
            print(f"Error retrieving Clang version: Unable to parse version string.")
            return 0
    except Exception as e:
        print(f"Error retrieving Clang version: {e}")
        return 0

def TestTTL(program_name):
    os.environ["PYOPENCL_COMPILER_OUTPUT"] = "1"
    os.environ["PYOPENCL_CTX"] = "0"
    os.environ["PYOPENCL_NO_CACHE"] = "1"

    # Allow an environment variable to provide the TTL_INCLUDE_PATH, if not defined regular paths used.
    if "TTL_INCLUDE_PATH" in os.environ:
        ttl_include_path = "-I" + os.environ["TTL_INCLUDE_PATH"]
    else:
        ttl_include_path = ""

    if "CLANG_CMD" in os.environ:
        CLANG_CMD = os.environ["CLANG_CMD"]
    else:
        CLANG_CMD = "clang++"

    if GetClangVerson(CLANG_CMD) < 16:
        print("Clang version 16 or greater is required.")
        print("Please set the CLANG_CMD environment variable to point to a suitable clang++ compiler.")
        print("For example: export CLANG_CMD=clang++-16")
        exit(-1)

    # Allow an environment variable to provide the TTL_INCLUDE_PATH, if not defined regular paths used.
    if "TTL_EXTRA_DEFINES" in os.environ:
        ttl_extra_defines =  " " + os.environ["TTL_EXTRA_DEFINES"] + " "
    else:
        ttl_extra_defines = ""

    # For convenience remove any extension if it included.
    program_name = os.path.splitext(os.path.basename(program_name))[0]
    kernel_name = program_name + "_kernel"

    # For variation a number of tensor random sizes are used, then tiled with random tile sizes
    for test_tensor_type, test_tensor_size, test_tensor_specifier in list([ ('Lux', 4, '%u'),
                                                                            ('char', 1, '%c'), ('uint8_t', 1, '%c'),
                                                                           ('int16_t', 2, '%d'), ('uint16_t', 2, '%u'),
                                                                           ('int32_t',4, '%d'), ('uint32_t',4, '%u'),
                                                                           ('int64_t',8, '%ld'), ('uint64_t',8, '%lu')
                                                                          ]):
        for test_compute_type in list(["CROSS", "COPY"]):
            # We all love a special case TTL_dupex_simple_buffering.cpp does not support CROSS
            if kernel_name == "TTL_duplex_simple_buffering_kernel" and test_compute_type == "CROSS":
                continue
            
            program_name_type = program_name + "_" + test_compute_type + "_" + test_tensor_type + ".so"
            compile_string = (
                "rm -f "
                + program_name_type
                + "; " + CLANG_CMD + " "
                + ttl_include_path
                + " -DTEST_COMPUTE_TYPE="
                + test_compute_type
                + " -DTEST_TENSOR_TYPE="
                + test_tensor_type
                + " -DKERNEL_NAME="
                + kernel_name
                + " -DTEST_TENSOR_TYPE_SPECIFIER="
                + "\"\\\"" + test_tensor_specifier + "\\\"\""
                + " -DTTL_TARGET=c -fPIC -shared -o "
                + program_name_type
                + " "
                + program_name
                + ".cpp")
            os.system(compile_string)
            print(compile_string)

            print("Testing %s with %s Tensors" % (program_name, test_tensor_type))

            for tensor_width in random.sample(range(1, 125), 3):
                for tensor_height in random.sample(range(1, 125), 3):
                    for tile_width in [1, tensor_width] + random.sample(range(1, tensor_width + 30), 3):
                        for tile_height in [1, tensor_height] + random.sample(range(1, tensor_height + 30), 3):
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

                            getattr(c_lib, kernel_name)(
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

                                    if test_compute_type == "CROSS":
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
                                            "%s Failed at [%d, %d] %s != %s Tensor size [%d, %d], Tile size [%d, %d], Tensor type %s, Compute Type %s"
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
                                                test_compute_type,
                                            )
                                        )

                                        error = True

                            if error:
                                exit(-1)

                        print("%s Passed Tensor size [%d, %d] Tile size [%d, %d] Type %s Compute %s" %(program_name, tensor_width, tensor_height, tile_width, tile_height, test_tensor_type, test_compute_type))


if __name__ == "__main__":
    for program_name in sys.argv[1:]:
        TestTTL(program_name)
