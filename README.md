[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

![](doc/tensor_tiling_library_slim.png)

*This readme will shortly be followed by a drop of the library source for review
within the Khronos organization, with an intent to go public once that review
cycle is complete.*

- [Tensor Tiling Library](#tensor-tiling-library)
  - [Purpose](#purpose)
  - [Design Principles](#design-principles)

Tensor Tiling Library
=====================

Tensor & Tiling Library is an open-source library to enable the efficient 
tiling and compute with tensors.

Please reach out to chris.gearing@mobileye.com for more information.

Purpose
-------

The purpose of this software package is to provide a simple-to-use standardized
way of tiling tensors to allow performance optimization in heterogeneous
machines with heterogeneous architectures. Tiling is one of the
pillars of performance. The Tensor Tiling Library is intended to provide a
standard approach that can be enhanced over time to allow for increased
automation and performance. The library is meant to be general purpose and
usable for a wide range of architectures and platforms.

Contributions, inputs, and requests from the greater community are most welcome to
allow this library to develop in a direction that one day leads to
full automation of tiling.

Design Principles
-----------------

The Tensor Tiling Library is designed to be

-   **Transparent**: the types and functions of the library are all exposed and
    visible to the user; there are no hidden components. This helps clarify
    exactly what the library supports, and how.

-   **Modular**: the library provides several constructs that can be used
    separately or in combination. This includes a construct for regular tiling,
    a construct for importing and exporting single tiles, and a construct for
    pipelining a single or pairs of import/export transactions.

-   **Extensible**: any part of the library can be copied and modified locally;
    new parts can be added locally to the library. Modification and additions
    regarded as generally useful should be considered for inclusion in the
    library.

-   **Easy to use**: provides simple and easy to use patterns, all included in
    header-files only.
