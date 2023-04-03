#!/usr/bin/env python3

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

import numpy
import pyopencl as cl
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
        ttl_include_path = "-I /usr/local/include/"

    context = cl.create_some_context()
    queue = cl.CommandQueue(context)

    # For convenience remove the .cl extension if it included.
    program_name = os.path.splitext(program_name)[0]
    program = cl.Program(context, open(program_name+'.cl').read()).build(options=ttl_include_path + " -D__TTL_VERSION__=04 -DTTL_COPY_3D")

    # For variation a number of tensor random sizes are used, then tiled with random tile sizes
    for tensor_width in random.sample(range(1, 125), 3):
        for tensor_height in random.sample(range(1, 125), 3):

            input_data = numpy.random.randint(256, size = tensor_width * tensor_height).astype(numpy.uint8)
            output_data = numpy.random.randint(256, size = tensor_width * tensor_height).astype(numpy.uint8)

            for i in range(0, tensor_height):
                for j in range(0, tensor_width):
                    input_data[i * tensor_width + j] = j

            input_buffer = cl.Buffer(context, cl.mem_flags.READ_ONLY  | cl.mem_flags.COPY_HOST_PTR , hostbuf=input_data)
            output_buffer = cl.Buffer(context, cl.mem_flags.READ_WRITE, output_data.nbytes)

            for tile_width in [1, tensor_width] + random.sample(range(1, tensor_width+30), 3):
                for tile_height in [1, tensor_height] + random.sample(range(1, tensor_height+30), 3):
                    error = False

                    getattr(program, program_name)(queue, (1,), None, input_buffer, numpy.int32(tensor_width), output_buffer, numpy.int32(tensor_width), numpy.int32(tensor_width), numpy.int32(tensor_height), numpy.int32(tile_width), numpy.int32(tile_height))

                    cl.enqueue_copy(queue, output_data, output_buffer)

                    for i in range(0, tensor_height):
                        for j in range(0, tensor_width):
                            expected = 0

                            if j > 0:
                                expected += input_data[i * tensor_width + (j - 1)];
                            if i  > 0:
                                expected += input_data[(i -1 )* tensor_width + j];

                            expected += input_data[i * tensor_width + j];

                            if j < (tensor_width - 1):
                                expected += input_data[i * tensor_width + (j + 1)]
                            if i < (tensor_height - 1):
                                expected += input_data[(i + 1) * tensor_width + j]
                                
                            expected &= 0xff

                            if output_data[i * tensor_width + j] != expected:
                                print("%s Failed at [%d, %d] %d != %d Tensor size [%d, %d], Tile size [%d, %d]" %(program_name, j, i, output_data[i * tensor_width + j], expected, tensor_width, tensor_height, tile_width, tile_height))
                                error = True
                                exit(-1)
                    
                    if error:
                        exit(-1)

                    print("%s Passed Tensor size [%d, %d] Tile size [%d, %d]" %(program_name, tensor_width, tensor_height, tile_width, tile_height))

if __name__ == '__main__':
    for program_name in sys.argv[1:]:
        TestTTL(program_name)
