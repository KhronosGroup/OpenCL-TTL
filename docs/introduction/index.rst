Introduction
============

Code developed for devices that process data exceeding the capacity of
their local memory must be partitioned into a series of stages, each of which
processes data that fits in the available local memory. Even if local memory is large
enough to accommodate all data in one single stage, it may be beneficial to
partition code into multiple stages to better cope with the latency of copying
data into and out of local memory. Such copying can be done asynchronously in OpenCL
via ``async_copy()`` builtin functions, which we refer to as ``import`` and ``export``.


Code for accelerators is often partitioned manually, resulting in code that is
in general difficult and cumbersome to write, hard to read, share, optimize, and
maintain.

The Tensor & Tiling Library is designed to
provide **transparent,** **modular**, and **extensible** **building-blocks** in **C**,
to support developing code for local memory based accelerators, and in general
where multi-dimensional tensors appear and are potentially tiled and pipelined.


Example
-------

As a preliminary example, Tensor Tiling Library can be used as follows to tile a
trivial a[i][j]=b[i][j]+1 kernel:

.. code-block:: c

   #include "TTL.h"

   #define TILE_WIDTH 100
   #define TILE_HEIGHT 100
   #define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

   void compute(TTL_int_tensor_t t_in, TTL_int_tensor_t t_out) {
      __local uchar *l_in = t_in.base;
      __local uchar *l_out = t_out.base;

      for (unsigned int y = 0; y < t_in.shape.height; y++) {
         for (unsigned int x = 0; x < t_in.shape.width; x++) {
            int idx_out = y * t_out.layout.total_row_length + x;
            int idx_in = y * t_in.layout.total_row_length + x;
            l_out[idx_out] = l_in[idx_in] + 1;
         }
      }
   }

   __kernel void add_one(__global char *restrict input_image, __global char *restrict output_image,
                        int image_width, int image_height, int image_stride)
   {
      __local uchar l_in[TILE_SIZE];
      __local uchar l_out[TILE_SIZE];
      
      // Regular tiling depends only on geometry aka logical tensors:
      TTL_shape_t image_shape = TTL_create_shape(image_width, image_height);
      TTL_shape_t tile_shape = TTL_create_shape(TILE_WIDTH, TILE_HEIGHT);
      TTL_tiler_t tiler = TTL_create_tiler(image_shape, tile_shape);
      
      // Accessing memory depends on layout aka physical tensors, where strides are absolute:
      TTL_layout_t external_layout = TTL_create_layout(image_stride);
      
      for (int tile_id = 0; tile_id < TTL_number_of_tiles(tiler); ++tile_id) {
         TTL_tile_t tile = TTL_get_tile(tile_id, tiler);
         TTL_int_tensor_t import_to = TTL_create_int_tensor(l_in, tile);
         TTL_int_tensor_t export_from = TTL_create_int_tensor(l_out, tile);
         TTL_ext_tensor_t import_from = TTL_create_ext_tensor(input_image, tile, external_layout);
         TTL_ext_tensor_t export_to = TTL_create_ext_tensor(output_image, tile, external_layout);

         TTL_blocking_import(import_to, import_from);
         compute(import_to, export_from);
         TTL_blocking_export(export_from, export_to);
      }
   }