Introduction to the Tensor & Tiling Library (TTL)
=================================================

CONTENTS:
---------

- [Introduction to the Tensor \& Tiling Library (TTL)](#introduction-to-the-tensor--tiling-library-ttl)
  - [CONTENTS:](#contents)
  - [Introduction](#introduction)
    - [Example](#example)
  - [Background](#background)
    - [Tiling](#tiling)
    - [Design Principles](#design-principles)
  - [TTL Logical Tiling](#ttl-logical-tiling)
    - [TTL\_create\_shape](#ttl_create_shape)
    - [TTL\_create\_overlap](#ttl_create_overlap)
    - [TTL\_tile\_t](#ttl_tile_t)
    - [TTL\_tiler\_t](#ttl_tiler_t)
  - [TTL Physical Tensors](#ttl-physical-tensors)
    - [TTL\_create\_layout](#ttl_create_layout)
    - [TTL\_\[const\]\_\[int,ext\]\_\[sub\]\_tensor\_t](#ttl_const_intext_sub_tensor_t)
    - [TTL\_io\_tensors](#ttl_io_tensors)
  - [TTL Import and Export Transactions](#ttl-import-and-export-transactions)
  - [TTL Pipelining Schemes](#ttl-pipelining-schemes)
    - [Pipelining Iterations](#pipelining-iterations)
    - [Duplex Buffering](#duplex-buffering)
    - [Double Buffering](#double-buffering)
    - [Simplex Buffering](#simplex-buffering)
  - [TTL Tiling Loop Parallelization](#ttl-tiling-loop-parallelization)
  - [Debugging](#debugging)
  - [Tiling Code Examples](#tiling-code-examples)
    - [Duplex Buffering Scheme](#duplex-buffering-scheme)
    - [Double Buffering Scheme](#double-buffering-scheme)
    - [Simplex Buffering Scheme](#simplex-buffering-scheme)
    - [Overlapped Tiler](#overlapped-tiler)
    - [Parallelizing Tiling Loop](#parallelizing-tiling-loop)
    - [Manual Double Buffering: dxDMA](#manual-double-buffering-dxdma)

Introduction
------------

Code developed for devices that process data exceeding the capacity of
their local memory must be partitioned into a series of stages, each of which
processes data that fits in the available local memory. Even if local memory is large
enough to accommodate all data in one single stage, it may be beneficial to
partition code into multiple stages to better cope with the latency of copying
data into and out of local memory. Such copying can be done asynchronously in OpenCL
via `async_copy()` builtin functions, which we refer to as `import` and `export`.


Code for accelerators is often partitioned manually, resulting in code that is
in general difficult and cumbersome to write, hard to read, share, optimize, and
maintain.

The Tensor & Tiling Library is designed to
provide **transparent,** **modular**, and **extensible** **building-blocks** in **C**,
to support developing code for local memory based accelerators, and in general
where multi-dimensional tensors appear and are potentially tiled and pipelined.


### Example

As a preliminary example, Tensor Tiling Library can be used as follows to tile a
trivial a[i][j]=b[i][j]+1 kernel:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Background
----------

### Tiling

The **data** accessed by a function can be made to fit in local memory
by **partitioning its control-flow** into stages, such that the data accessed by
each stage fits in local memory. In all cases considered, this partitioning takes
the form *Loop Tiling*, where an initial unfitting loop nest such as:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 for (int y = 0; y < TooManyRows; ++y)
   for (int x = 0; x < TooManyColumns; ++x)
     A[y * StrideA + x] = B[y * StrideB + x] + 1;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

is strip-mined/blocked/tiled in (either or) both loops to produce:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 for (int y = 0; y < TooManyRows; y += TileY)
   for (int x = 0; x < TooManyColumns; x += TileX) {
     // Start of stage.
     for (int yy = y; yy < min(y + TileY , TooManyRows); ++yy)
       for (int xx = x; xx < min(x + TileX, TooManyColumns); ++xx)
         A[yy * StrideA + xx] = B[yy * StrideB + xx] + 1;
     // End of stage.
   }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Loop tiling produces an outer-loop, in the above case a doubly-nested
outer-loop, that iterates over stages or tiles. These outer loops could be
represented as:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 for (int tile_id = 0; tile_id < number_of_tiles; ++tile_id) {
   // Derive y, x, and tile parameters from tile_id.
 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Notice that both arrays A and B in the above example are tiled together. Arrays
with multiple accesses of distinct offsets, such as in stencil access patterns,
result in tiles that overlap. Arrays that are accessed with non-unit strides
should be allocated increased memory accordingly along the x dimension - as if
their element size has increased.

This transformation partitions or tiles the *Iteration Space* of a nested loop,
thereby also partitioning or tiling the *Memory Space* of each array accessed in
the loop. Tiling traditionally involves a single (usually 2D) tile size which
applies to the outer-loop of a loop-nest in terms of iterations, and thereby to
all tiled arrays of the loop in terms of elements. It can be extended to
multiple loops by conceptually fusing them together. Each array however may
require a distinct tile size due to its unique access pattern, and to be bumped
by a unique stride along a distinct loop dimension.

Once a loop and the arrays it accesses have been tiled, the operations involved
with copying the data of each tile can be double buffered and pipelined
across loop iterations, thereby overlapping data communication with computation,
and overlapping asynchronous imports and exports.

### Design Principles

The Tensor Tiling Library is designed to be

-   **Transparent**: the types and functions of the library are all exposed and
    visible to the user; there are no hidden components. This helps clarify
    exactly what the library supports, and how.

-   **Modular**: the library provides several constructs that can be used
    separately or in combination. This includes a construct for tensors, for
    regular tiling, a construct for importing and exporting single tiles, and a
    construct for pipelining a single or pairs of import/export transactions.

-   **Extensible**: any part of the library can be copied and modified locally;
    new parts can be added locally to the library. Modification and additions
    regarded as generally useful should be considered for inclusion in the
    library.

-   **Easy to use**: provides simple and easy to use patterns, all included in
    header-files only.

A *tile* is considered to be a memory region that can be copied asynchronously.
Therefore, a tile is in general a 3-dimensional *tensor* of elements, embedded
within an enclosing memory layout.

In TTL tiles are defined with optional overlap in every dimension (see next
section). For example, the following figure shows the tiling along x-dimension
produced by a tiler with 2D space shape of (5, 250), 2D tile shape of (5, 5),
and **overlap.width**=1, so that every pair of horizontally-adjacent tiles has
one column of elements in common:

![](overlap.jpg)

The following figure shows all 15 tiles produced by a non-overlapping tiler with
2D space shape of {900, 800} and 2D tile shape of {200, 300}.

Note that tiles appearing last in each dimension are of smaller size - the
remainder of dividing 900 by 200 and 800 by 300. Each tile has a unique ID from
zero to **number_of_tiles**-1, following row-major or column-major order:

![](tiling_ttl.png)

The following sections describe the layers of API provided by TTL.

TTL Logical Tiling
------------------

The first layer of TTL deals with logical tiling of 3D shapes. The basic unit of
these shapes - an "element" - is independent of its actual size or location in
memory, hence the term "logical". The associated "physical" aspects of size and
location in memory are dealt with separately.

### TTL_create_shape

TTL_shape_t defines the number of elements along each dimension in a 3D box:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
  TTL_dim_t width;  // Number of elements along dimension x.
  TTL_dim_t height; // Number of rows along dimension y
  TTL_dim_t depth;  // Number of planes along dimension z
} TTL_shape_t;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TTL_create_shape() APIs define shapes of desired dimensions, complementing
remaining dimensions with '1's:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TTL_shape_t TTL_create_shape(TTL_dim_t width);
TTL_shape_t TTL_create_shape(TTL_dim_t width, TTL_dim_t height);
TTL_shape_t TTL_create_shape(TTL_dim_t width, TTL_dim_t height, TTL_dim_t depth);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A "Big" 3D box can be partitioned into pairwise disjointed "Small" 3D boxes,
simply by defining the two boxes as TTL_shape_t's and using them to construct a
TTL_tiler_t:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TTL_tiler_t TTL_create_tiler(TTL_shape_t Big, TTL_shape_t Small);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This TTL_tiler_t can then be used to traverse all the parts of Big's partition
(referred to as "tiles") each of size Small (or smaller), as explained below.

A Big 3D box can be partitioned into overlapping Small 3D boxes - where every
pair of adjacent parts share a fixed number of elements along each dimension,
using the following constructs:

### TTL_create_overlap

TTL_overlap_t defines the number of elements shared between two adjacent tiles
along each dimension of a 3D box:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
  TTL_overlap_dim_t width;
  TTL_overlap_dim_t height;
  TTL_overlap_dim_t depth;
} TTL_overlap_t;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TTL_create_overlap() APIs define the overlaps along the desired dimensions,
complementing remaining dimensions with '0's:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TTL_overlap_t TTL_create_overlap(TTL_overlap_dim_t width);
TTL_overlap_t TTL_create_overlap(TTL_overlap_dim_t width, TTL_overlap_dim_t height);
TTL_overlap_t TTL_create_overlap(TTL_overlap_dim_t width, TTL_overlap_dim_t height, TTL_overlap_dim_t depth);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An overlap can then be used together with Big and Small to create the desired
overlapping tiler:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TTL_tiler_t TTL_create_overlap_tiler(TTL_shape_t Big, TTL_shape_t Small, TTL_overlap_t Overlap);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### TTL_tile_t

TTL_tile_t defines the shape and position of each part in a partitioning
produced by TTL_tiler_t:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
  TTL_shape_t shape;
  TTL_shape_t offset; // In terms of number of elements
} TTL_tile_t;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The offset defines where each tile "starts" and is therefore distinct across the
tiles of a tiler, with a first tile typically starting at offset zero. The
shapes are typically equal to the Small shape provided to the tiler, except for
last tiles along each dimension which may be smaller. A tile having zero shape
represents an empty or out-of-range tile.

### TTL_tiler_t

TTL_tiler_t provides the following APIs:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int TTL_number_of_tiles(TTL_tiler_t t);       // Total number of tiles
TTL_dim_t TTL_tiles_in_width(TTL_tiler_t t);  // Number of tiles in width
TTL_dim_t TTL_tiles_in_height(TTL_tiler_t t); // Number of tiles in height
TTL_dim_t TTL_tiles_in_depth(TTL_tiler_t t);  // Number of tiles in depth
TTL_tile_t TTL_get_tile(int tile_id, TTL_tiler_t *t); // Return tile number tile_id, empty if tile_id is out of range
int TTL_valid_tile_id(int tile_id, TTL_tiler_t t);    // Check if tile_id is in range
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TTL Physical Tensors
--------------------

The second layer of TTL deals with projecting or laying out logical tiles onto
physical memory spaces.

Note that separating logical tiling from memory considerations facilitates
reusing the same TTL_tiler_t to tile distinct images (located in different
memory addresses) with potentially distinct element sizes and memory alignments,
provided they contain the same number and shape of elements to be tiled the same
way.

### TTL_create_layout

Each tile is embedded in global and local memories within some enclosing shape,
e.g., to account for possible alignment padding. This embedding is referred to
as *layout*, which specifies the spacing between the start of consecutive rows in
units of elements, and spacing of the between start of consecutive planes in units
of elements.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
    TTL_dim_t row_spacing;    ///< The distance between the start of consequtive rows in units of elements.
    TTL_dim_t plane_spacing;  ///< The distance between the start of consequtive planes in units of elements.
} TTL_layout_t;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TTL_create_layout() APIs define the layouts along the desired dimensions:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TTL_layout_t TTL_create_layout();
TTL_layout_t TTL_create_layout(TTL_dim_t total_row_length);
TTL_layout_t TTL_create_layout(TTL_dim_t total_row_length, TTL_dim_t total_number_of_rows);

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### TTL_ type family tensor_t

TTL_[const]\_[int/ext]\_[sub]_tensor_t combine the logical dimensions of a tile
along with its physical mapping to memory. The two constructs allow the creation
of local [int] and global [ext] versions with a const attribute. The
TTL_tensor_t structs contain all the information needed for issuing an import or
export transaction, and for reading and writing to the tensor.

As well as usage for tiling the tensors should also be passed to compute
functions. The type contains all the data needed to read from a tile that was
imported and write to a tile before it is exported.
An external tensor can also be passed to the kernel as it contains all data
needed for tiling, importing and exporting.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
  [const] __[local/global] void *base;
  TTL_dim_t elem_size;
  TTL_layout_t layout;
  TTL_shape_t shape;
#if sub
  struct {
    TTL_shape_t shape;
    TTL_offset_t sub_offset;
    } origin;
#endif
} TTL_[const]_[ext/int]_[sub]_tensor_t ;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This defines the following eight types of TTL_tensor_t's:

TTL_int_tensor_t;  
TTL_const_int_tensor_t;  
TTL_int_sub_tensor_t;  
TTL_const_int_sub_tensor_t;  
TTL_ext_tensor_t;  
TTL_const_ext_tensor_t;  
TTL_ext_sub_tensor_t;  
TTL_const_ext_sub_tensor_t;

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Construct a buffer with default, packed layout, whose strides match the provided shape and element size w/o padding.
// Element size is implicitly set to gentype size, where gentype represents any type amenable to sizeof, including but not restricted to OpenCL vector types, structs, but excluding void.
TTL_int_tensor_t TTL_create_int_tensor(__local gentype *base, TTL_tile_t tile);

// Element size is implicitly set to gentype size, where gentype represents any type amenable to sizeof, including but not restricted to OpenCL vector types, structs, but excluding void.
TTL_int_tensor_t TTL_create_int_tensor(__local gentype *base, TTL_tile_t tile, TTL_layout_t int_layout);

// Explicit layout and element size
TTL_int_tensor_t TTL_create_int_tensor(__local gentype *base, TTL_tile_t tile, TTL_layout_t int_layout, int elem_size);

// Element size is implicitly set to gentype size, where gentype represents any type amenable to sizeof, including but not restricted to OpenCL vector types, structs, but excluding void.
TTL_ext_tensor_t TTL_create_ext_tensor(__global gentype *base, TTL_tile_t tile, TTL_layout_t ext_layout);

// Explicit layout and element size
TTL_ext_tensor_t TTL_create_ext_tensor(__global gentype *base, TTL_tile_t tile, TTL_layout_t ext_layout, int elem_size);

// Returns 1 if an internal tensor 'tensor' is valid, i.e. has a non-empty shape. Otherwise, returns 0.
// Useful when prolog/epilogs creates non-valid tensors.
int TTL_valid_int_tensor(TTL_int_tensor_t tensor);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Explicit layouts and/or element size can be provided by overriding the default
values. The API can also be extended with constructors for explicit sizes, if
needed.

### TTL_io_tensors

TTL_io_tensors_t holds two internal tensors ready for processing: imported_to as
input tensor and to_export_from as output tensor:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
  TTL_int_tensor_t imported_to;
  TTL_int_tensor_t to_export_from;
} TTL_io_tensors_t;

// Returns true if tensors are valid.
// Useful when prolog/epilogs creates non-valid tensors.
int TTL_tensors_empty(TTL_io_tensors_t tensors);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TTL Import and Export Transactions
----------------------------------

The third layer of TTL deals with transactions that copy tensors from global to
local memory and back, referred to as import and export, respectively. These
transactions are asynchronous and correspond to async_work_group_copy() builtin
functions of OpenCL and their use of "event_t". Similar to OpenCL, in TTL one
event_t can serve multiple transactions, and it is possible to wait on multiple
events. Unlike OpenCL, every import and export in TTL must be provided a
non-null event_t, which can be produced by TTL_get_event().

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TTL_event_t TTL_get_event(); // Initialize an event.

// Import the data in external_tensor to internal_tensor.
// The transaction is added to the event e.
void TTL_import(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor, TTL_event_t *e);

// Export the data in internal_tensor to external_tensor.
// The transaction is added to the event e.
void TTL_export(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor, TTL_event_t *e);

void TTL_wait(int num_events, TTL_event_t *events); // Wait for first num_events in events array
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TTL_blocking_import/export can be used to issue a blocking transaction, i.e.,
get an event, issue a transaction and immediately wait for its completion:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void TTL_blocking_import(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor);
void TTL_blocking_export(TTL_int_tensor_t internal_tensor, TTL_ext_tensor_t external_tensor);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
