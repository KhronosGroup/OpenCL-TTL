[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

<p align="center"><img width="30%" src="doc/tensor_tiling_library.png" /></p>

# Tensor Tiling Library

Tensor & Tiling Library is an open-source library to enable the efficient tiling and compute with tensors.

Please reach out to chris.gearing@mobileye.com or ayal.zaks@mobileye.com for more information.

This document outlines the purpose of this sample implementation as well as provide build and execution instructions.

## CONTENTS: <!-- omit in toc -->

- [Tensor Tiling Library](#tensor-tiling-library)
  - [Purpose](#purpose)
  - [Example Source](#example-source)
  - [Building And Executing](#building-and-executing)
    - [CMake](#cmake)
      - [Tested Supported systems:](#tested-supported-systems)
      - [Pre-requisite:](#pre-requisite)
      - [Building Samples:](#building-samples)
      - [Install:](#install)
  - [Included Unit Tests](#included-unit-tests)
  - [Bug Reporting](#bug-reporting)

## Purpose

The purpose of this software package is to provide a simple to use standardized way of tiling tensors to allow performance optimization in heterogenous machines. Tiling is one of the pillars of performance and the Tensor Tiling Library is intended to provide a standardized approach.

The library is intended to be general purpose and usable on all architectures.

Currently the Tensor Tiling Library is:

* passing its own unit tests
* contains reference implementations
* optimized

## Example Source

This is a double tiling example where the data is simultaneously moved from host<->device whilst the
compute is occurring.

```
#include "TTL/TTL.h"

__kernel void TTL_double_buffering(__global uchar *restrict ext_base_in, int external_stride_in,
                                   __global uchar *restrict ext_base_out, int external_stride_out, int width,
                                   int height, int tile_width, int tile_height) {
    __local uchar l_in1[MEMSZ];
    __local uchar l_in2[MEMSZ];
    __local uchar l_out1[MEMSZ];
    __local uchar l_out2[MEMSZ];

    // Logical Tiling
    const TTL_shape_t global_tensor = TTL_create_shape(width, height);
    const TTL_shape_t tile_shape = TTL_create_shape(tile_width, tile_height);
    const TTL_tiler_t tiler = TTL_create_tiler(global_tensor, tile_shape);

    // External layouts.
    const TTL_layout_t ext_layout_in = TTL_create_layout(external_stride_in);
    const TTL_layout_t ext_layout_out = TTL_create_layout(external_stride_out);

    const TTL_const_ext_tensor_t ext_input_tensor =
        TTL_create_const_ext_tensor(ext_base_in, global_tensor, ext_layout_in);
    const TTL_ext_tensor_t ext_output_tensor = TTL_create_ext_tensor(ext_base_out, global_tensor, ext_layout_out);

    // TTL_start_import_double_buffering will being the import of the first tile
    TTL_event_t import_DB_e = TTL_get_event();
    TTL_import_double_buffering_t import_db = TTL_start_import_double_buffering(
        l_in1, l_in2, ext_input_tensor, &import_DB_e, TTL_get_tile(0, tiler));

    TTL_event_t export_DB_e = TTL_get_event();
    TTL_export_double_buffering_t export_db =
        TTL_start_export_double_buffering(l_out1, l_out2, ext_output_tensor, &export_DB_e);

    for (int i = 0; i < TTL_number_of_tiles(tiler); ++i) {
        TTL_tile_t tile_next_import = TTL_get_tile(i + 1, tiler);
        TTL_tile_t tile_current_export = TTL_get_tile(i, tiler);

        // These wait for the current transfers to complete, and begin the next
        TTL_int_sub_tensor_t imported_to = TTL_step_buffering(&import_db, tile_next_import);
        TTL_int_sub_tensor_t exported_from = TTL_step_buffering(&export_db, tile_current_export);

        // Compute whilst the transfers are taking place (on separate buffers)
        compute(imported_to, exported_from);
    }

    // These wait for the last transfers to complete.
    TTL_finish_import_double_buffering(&import_db);
    TTL_finish_export_double_buffering(&export_db);
}
```

## Building And Executing

The sample implementation builds under POCL on x86 and demonstrates a number of buffering schemes.

It builds using cmake and has been tested on Linux

### CMake

#### Tested Supported systems:

* Linux

#### Pre-requisite:

* python 2.x (Tested with python 2.7)
* CMAKE 2.8.12 or higher. (should be in PATH)

#### Building Samples:

    Install Pocl http://portablecl.org/docs/html/install.html or other environment of your choice.
    Install pyopencl https://pypi.org/project/pyopencl/

    Linux:
    ------
    From shell:

    > cd opencl/samples
    > ./ttl_sample_runner.py *.cl
    > cd ../c/samples
    > ./ttl_sample_runner.py *.c

#### Install:

    See INSTALL file

## Included Unit Tests

    See opencl/test/README.md


## Bug Reporting

Bug reports can be reported by filing an [issue on the GitHub](https://github.com/KhronosGroup/OpenCL-TTL/issues)
