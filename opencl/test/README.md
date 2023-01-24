# Tensor Tiling Library (TTL) Tests (TTLT)

This it the Tests for all versions of the [Tensor Tiling Library](https://www.khronos.org/opencl/).

## Basis

These tests are heavily based on the [OpenCL-CTS](https://github.com/KhronosGroup/OpenCL-CTS.git) framework.
This document is a derivative of the README.md in the OpenCL-CTS repository.

## Building the Tests

The Tests support Linux. In particular, GitHub Actions CI builds against Ubuntu 20.04.

Compiling the TTLT requires the following CMake configuration options to be set:

* `CL_INCLUDE_DIR` Points to the unified
  [OpenCL-Headers](https://github.com/KhronosGroup/OpenCL-Headers).
* `CL_LIB_DIR` Directory containing the OpenCL library to build against.
* `OPENCL_LIBRARIES` Name of the OpenCL library to link.
* `CLConform_SOURCE_DIR` Directory containing the OpenCL conformance library

It is advised that the [OpenCL ICD-Loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
is used as the OpenCL library to build against. Where `CL_LIB_DIR` points to a
build of the ICD loader and `OPENCL_LIBRARIES` is "OpenCL".

### Example Build

Steps on a Linux platform to clone dependencies from GitHub sources, configure
a build, and compile.

```sh
git clone https://github.com/KhronosGroup/OpenCL-CTS.git
git clone https://github.com/KhronosGroup/OpenCL-Headers.git
git clone https://github.com/KhronosGroup/OpenCL-ICD-Loader.git
git clone https://github.com/KhronosGroup/ttl.git

mkdir OpenCL-ICD-Loader/build
cmake -S OpenCL-ICD-Loader -B OpenCL-ICD-Loader/build \
      -DOPENCL_ICD_LOADER_HEADERS_DIR=$PWD/OpenCL-Headers
cmake --build ./OpenCL-ICD-Loader/build --config Release

export TTL_TEST_DIR=$PWD/ttl/opencl/test
mkdir $TTL_TEST_DIR/build
cmake -S $TTL_TEST_DIR -B $TTL_TEST_DIR/build \
      -DCL_INCLUDE_DIR=$PWD/OpenCL-Headers \
      -DCL_LIB_DIR=$PWD/OpenCL-ICD-Loader/build \
      -DOPENCL_LIBRARIES=OpenCL \
      -DCLConform_SOURCE_DIR=$PWD/OpenCL-CTS \
      -DTTL_INSTALL_DIR=$PWD/ttl
cmake --build $TTL_TEST_DIR/build --config Release
```

__Note__: TTL_INSTALL_DIR is optional and will default to the system default. If
TTL has been installed then TTL_INSTALL_DIR is not required.

## Running the TTLT Tests

A build of the TTLT contains a single executable representing the key functionality
of the TTL. 

```sh
ttl/opencl/test/build/test_ttl ttl
```

See the `--help` output for the list of sub-tests available, as well as other options
for configuring execution.

If the OpenCL library built against is the ICD Loader, and the vendor library to
be tested is not registered in the
[default ICD Loader location](https://github.com/KhronosGroup/OpenCL-ICD-Loader#registering-icds)
then the [OCL_ICD_FILENAMES](https://github.com/KhronosGroup/OpenCL-ICD-Loader#table-of-debug-environment-variables)
environment variable will need to be set for the ICD Loader to detect the OpenCL
library to use at runtime. For example, to run the basic tests on a Linux
platform:
