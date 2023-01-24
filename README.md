[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

<p align="center"><img width="30%" src="doc/tensor_tiling_library.png" /></p>

# Tensor Tiling Library

Tensor & Tiling Library is an open-source library to enable the efficient tiling and compute with tensors.

Please reach out to chris.gearing@mobileye.com or ayal.zaks@mobileye.com for more information.

This document outlines the purpose of this sample implementation as well as provide build and execution instructions.

## CONTENTS: <!-- omit in toc -->

- [Tensor Tiling Library](#tensor-tiling-library)
  - [Purpose](#purpose)
  - [Building And Executing](#building-and-executing)
    - [CMake](#cmake)
      - [Supported systems:](#supported-systems)
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
