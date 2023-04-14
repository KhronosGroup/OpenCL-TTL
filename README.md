<h1 align="center">Tensor Tiling Library</h1>
<p align="center">
  <img width="30%" src="doc/tensor_tiling_library.png" />
</p>
<p align="center">
  <a href="https://opensource.org/licenses/Apache-2.0">
    <img src="https://img.shields.io/badge/license-Apache%20-blue.svg" alt="License">
  </a>
  <a href="https://github.com/KhronosGroup/OpenCL-TTL/issues">
    <img src="https://img.shields.io/github/issues/KhronosGroup/OpenCL-TTL" alt="Issues">
  </a>
</p>

Tensor & Tiling Library is an open-source library to enable the efficient tiling and compute with tensors.

Please reach out to chris.gearing@mobileye.com or ayal.zaks@mobileye.com for more information.

This document outlines the purpose of this sample implementation and provides build and execution instructions.

## Contents <!-- omit in toc -->

- [Tensor Tiling Library](#tensor-tiling-library)
  - [Purpose](#purpose)
  - [Example](#example)
  - [Building And Executing](#building-and-executing)
    - [CMake](#cmake)
      - [Tested Supported Systems](#tested-supported-systems)
      - [Requirements](#requirements)
      - [Building the Samples](#building-the-samples)
      - [Installation](#installation)
  - [Included Unit Tests](#included-unit-tests)
  - [Bug Reporting](#bug-reporting)

## Purpose

The purpose of this software package is to provide a simple to use standardized way of tiling tensors to allow performance optimization in heterogenous machines. Tiling is one of the pillars of performance and the Tensor Tiling Library is intended to provide a standardized approach.

The library is intended to be general purpose and usable on all architectures.

Currently the Tensor Tiling Library:

* passes its own unit tests
* contains reference implementations
* is optimized

## Example

This is a double tiling example where the data is simultaneously moved from host<->device whilst the
compute is occurring.

```c
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
    TTL_finish_buffering(&import_db);
    TTL_finish_buffering(&export_db);
}
```

## Building And Executing

The sample implementation builds under POCL on x86 and demonstrates a number of buffering schemes.

It builds using CMake and has been tested on Linux.

### CMake

#### Tested Supported Systems

* Linux

#### Requirements

* Python 2.x (Tested with Python 2.7)
* CMake 2.8.12 or higher (should be in PATH)

#### Building the Samples

- Install [PoCL](http://portablecl.org/docs/html/install.html) or another environment of your choice.
- Install [pyopencl](https://pypi.org/project/pyopencl/)

Execute the following commands:

```sh
$ cd opencl/samples
$ ./TTL_sample_runner.py *.cl
$ cd ../c/samples
$ ./TTL_sample_runner.py *.c
```

#### Installation

See [INSTALL](./INSTALL).

## Included Unit Tests

See [the test README](./opencl/test/).


## Bug Reporting

Bug reports can be reported by filing an [issue on GitHub](https://github.com/KhronosGroup/OpenCL-TTL/issues).
