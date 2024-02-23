# C Samples for the Tensor Tiling Library (TTL)

These C Samples allow for a host only build, debug, and validation of the TTL library. A key ingredient is the ease of build and ease of debug.

## Basis

The samples can be built and testing using either a C wrapper of a Python wrapper.

## Executing the Samples

Install the TTL include files as described in [INSTALL](../../INSTALL)

## Python Wrapper

    export TTL_INCLUDE_PATH=[PATH TO TTL]
    export TTL_EXTRA_DEFINES=[ANY EXTRA DEFINES]
    ./TTL_sample_runner.py TTL_double_buffering.c

The name can be wildcarded

    export TTL_INCLUDE_PATH=[PATH TO TTL]
    export TTL_EXTRA_DEFINES=[ANY EXTRA DEFINES]
    ./TTL_sample_runner.py TTL_*.c

TTL_EXTRA_DEFINES can for example define __TTL_DEBUG=1 to provide additional
debug output.

    export TTL_EXTRA_DEFINES="__TTL_DEBUG=1"

## C Wrapper

    export TTL_INCLUDE_PATH=[PATH TO TTL]
    clang -Wextra -Wall -DKERNEL_NAME=TTL_duplex_buffering -DEVERY_N_LINES=1 -DTEST_TENSOR_TYPE_SPECIFIER=\"%d\" -DTEST_TENSOR_TYPE=int -I $TTL_INCLUDE_PATH -DTTL_TARGET=c -g -O0 main.c TTL_duplex_buffering.c -o c_test
    ./c_test

## The "Kernel"

The kernel is a simple sum of a cross of the input.

Given an input tensor I of size X * Y

The output tensor O is

    O[0,0] = I[0,0]+I[-1,0]+I[+1,0]+I[0,+1]+I[0,-1]

or

    for y = 0 to Y:
      for x = 0 to X:
        O[y,x] = I[y,x]+I[y-1,x]+I[y+1,x]+I[y,x+1]+I[y,x-1]

At the edges of the input [-1,0] for example a zero value is augmented meaning that the input tensor the kernel receives
is effectively bigger than the actual input tensor with the augmentation being zero values.

The TTL_sample_runner.py based tests run through a random set of tensor and tile sizes, with an augmentation of 1 - providing
a fairly broad-based testing.

The compute.h file which contains the calculations also contains a checker function. This checker function and a checker
function in the ./TTL_sample_runner.py file validate the tiled calculations.

