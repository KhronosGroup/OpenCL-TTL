Logical Tiling
==============

The first layer of TTL deals with logical tiling of 3D shapes. The basic unit of
these shapes - an "element" - is independent of its actual size or location in
memory, hence the term "logical". The associated "physical" aspects of size and
location in memory are dealt with separately.

TTL_create_shape
----------------

TTL_shape_t defines the number of elements along each dimension in a 3D box:

.. code-block:: c

  typedef struct {
    TTL_dim_t width;  // Number of elements along dimension x.
    TTL_dim_t height; // Number of rows along dimension y
    TTL_dim_t depth;  // Number of planes along dimension z
  } TTL_shape_t;


``TTL_create_shape()`` APIs define shapes of desired dimensions, complementing
remaining dimensions with '1's:

.. code-block:: c

  TTL_shape_t TTL_create_shape(TTL_dim_t width);
  TTL_shape_t TTL_create_shape(TTL_dim_t width, TTL_dim_t height);
  TTL_shape_t TTL_create_shape(TTL_dim_t width, TTL_dim_t height, TTL_dim_t depth);

A "Big" 3D box can be partitioned into pairwise disjointed "Small" 3D boxes,
simply by defining the two boxes as TTL_shape_t's and using them to construct a
TTL_tiler_t:

.. code-block:: c
  
  TTL_tiler_t TTL_create_tiler(TTL_shape_t Big, TTL_shape_t Small);

This TTL_tiler_t can then be used to traverse all the parts of Big's partition
(referred to as "tiles") each of size Small (or smaller), as explained below.

A Big 3D box can be partitioned into overlapping Small 3D boxes - where every
pair of adjacent parts share a fixed number of elements along each dimension,
using the following constructs:

TTL_create_overlap
------------------

``TTL_overlap_t`` defines the number of elements shared between two adjacent tiles
along each dimension of a 3D box:

.. code-block:: c

  typedef struct {
    TTL_overlap_dim_t width;
    TTL_overlap_dim_t height;
    TTL_overlap_dim_t depth;
  } TTL_overlap_t;

``TTL_create_overlap()`` APIs define the overlaps along the desired dimensions,
complementing remaining dimensions with '0's:

.. code-block:: c

  TTL_overlap_t TTL_create_overlap(TTL_overlap_dim_t width);
  TTL_overlap_t TTL_create_overlap(TTL_overlap_dim_t width, TTL_overlap_dim_t height);
  TTL_overlap_t TTL_create_overlap(TTL_overlap_dim_t width, TTL_overlap_dim_t height, TTL_overlap_dim_t depth);

An overlap can then be used together with Big and Small to create the desired
overlapping tiler:

.. code-block:: c
  
  TTL_tiler_t TTL_create_overlap_tiler(TTL_shape_t Big, TTL_shape_t Small, TTL_overlap_t Overlap);

TTL_tile_t
----------

``TTL_tile_t`` defines the shape and position of each part in a partitioning
produced by ``TTL_tiler_t``:

.. code-block:: c

  typedef struct {
    TTL_shape_t shape;
    TTL_shape_t offset; // In terms of number of elements
  } TTL_tile_t;

The offset defines where each tile "starts" and is therefore distinct across the
tiles of a tiler, with a first tile typically starting at offset zero. The
shapes are typically equal to the Small shape provided to the tiler, except for
last tiles along each dimension which may be smaller. A tile having zero shape
represents an empty or out-of-range tile.

TTL_tiler_t
-----------

``TTL_tiler_t`` provides the following APIs:

.. code-block:: c

  int TTL_number_of_tiles(TTL_tiler_t t);       // Total number of tiles
  TTL_dim_t TTL_tiles_in_width(TTL_tiler_t t);  // Number of tiles in width
  TTL_dim_t TTL_tiles_in_height(TTL_tiler_t t); // Number of tiles in height
  TTL_dim_t TTL_tiles_in_depth(TTL_tiler_t t);  // Number of tiles in depth
  TTL_tile_t TTL_get_tile(int tile_id, TTL_tiler_t *t); // Return tile number tile_id, empty if tile_id is out of range
  int TTL_valid_tile_id(int tile_id, TTL_tiler_t t);    // Check if tile_id is in range