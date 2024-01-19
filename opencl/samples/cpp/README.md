# CPP OpenCL Samples for the Tensor Tiling Library (TTL)

See opencl/samples/README.md for overview

## Basis

The samples can be built and testing using a CPP host executable

## CPP Wrapper

Two examples exist, the first is a overlapping tiling example, the second a simple tiling copy example.

A key to these samples is that Tensors are used on the Host and the Device side.

Compile as follows

```
export TTL_INCLUDE_PATH=[PATH TO TTL]
export OPEN_CL_INCLUDE_PATH=[PATH_TO_OPENCL_INCLUDE_FILES]
clang -O0 -g -D TTL_TARGET=c -D COMPUTE=square.h -D CL_TARGET_OPENCL_VERSION=300 -I $TTL_INCLUDE_PATH -I $OPEN_CL_INCLUDE_PATH -o TTL_sample_overlap TTL_sample_runner.cpp -lOpenCL -lstdc++
./TTL_sample_overlap
clang -O0 -g -D TTL_TARGET=c -D COMPUTE=cross.h -D CL_TARGET_OPENCL_VERSION=300 -I $TTL_INCLUDE_PATH -I $OPEN_CL_INCLUDE_PATH -o TTL_sample_overlap TTL_sample_runner.cpp -lOpenCL -lstdc++
./TTL_sample_overlap
clang -O0 -g -D TTL_TARGET=c -D COMPUTE=copy.h -D CL_TARGET_OPENCL_VERSION=300 -I $TTL_INCLUDE_PATH -I $OPEN_CL_INCLUDE_PATH -o TTL_sample_overlap TTL_sample_runner.cpp -lOpenCL -lstdc++
./TTL_sample_overlap
```

## The "Copy Kernel" 

The kernel simply copies the input buffer to the output buffer

## The "Cross Kernel"

The "Cross Kernel" is described in  opencl/samples/python/README.md

## The "Square Kernel"

The kernel is a simple sqr of the input to the output.

Given an input tensor I of size X * Y

The output tensor O is

    O[0,0] = I[0,0]^2

or

    for y = 0 to Y:
      for x = 0 to X:
        O[y,x] = I[y,x]^2


For a kernel two includes files exist - compute_XXXX.h and check_XXXX.h (XXXX is square or cross). The compute is used by the
kernel and the check is used by the host code