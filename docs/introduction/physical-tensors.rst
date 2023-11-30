Physical Tensors
================

The second layer of TTL deals with projecting or laying out logical tiles onto
physical memory spaces.

Note that separating logical tiling from memory considerations facilitates
reusing the same TTL_tiler_t to tile distinct images (located in different
memory addresses) with potentially distinct element sizes and memory alignments,
provided they contain the same number and shape of elements to be tiled the same
way.

TTL_create_layout
-----------------

Each tile is embedded in global and local memories within some enclosing shape,
e.g., to account for possible alignment padding. This embedding is referred to
as *layout*, which specifies the spacing between the start of consecutive rows in
units of elements, and spacing of the between start of consecutive planes in units
of elements.

.. code-block:: c

    typedef struct {
        TTL_dim_t row_spacing;    ///< The distance between the start of consequtive rows in units of elements.
        TTL_dim_t plane_spacing;  ///< The distance between the start of consequtive planes in units of elements.
    } TTL_layout_t;

TTL_create_layout() APIs define the layouts along the desired dimensions:

.. code-block:: c

    TTL_layout_t TTL_create_layout();
    TTL_layout_t TTL_create_layout(TTL_dim_t total_row_length);
    TTL_layout_t TTL_create_layout(TTL_dim_t total_row_length, TTL_dim_t total_number_of_rows);

TTL\_ type family tensor_t
--------------------------

TTL\_[const]\_[int/ext]\_[sub]\_tensor_t combine the logical dimensions of a tile
along with its physical mapping to memory. The two constructs allow the creation
of local [int] and global [ext] versions with a const attribute. The
TTL_tensor_t structs contain all the information needed for issuing an import or
export transaction, and for reading and writing to the tensor.

As well as usage for tiling the tensors should also be passed to compute
functions. The type contains all the data needed to read from a tile that was
imported and write to a tile before it is exported.
An external tensor can also be passed to the kernel as it contains all data
needed for tiling, importing and exporting.

.. code-block:: c

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

This defines the following eight types of TTL_tensor_t's:

- ``TTL_int_tensor_t``
- ``TTL_const_int_tensor_t``
- ``TTL_int_sub_tensor_t``
- ``TTL_const_int_sub_tensor_t``
- ``TTL_ext_tensor_t``
- ``TTL_const_ext_tensor_t``
- ``TTL_ext_sub_tensor_t``
- ``TTL_const_ext_sub_tensor_t``

.. code-block:: c

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

Explicit layouts and/or element size can be provided by overriding the default
values. The API can also be extended with constructors for explicit sizes, if
needed.

TTL_io_tensors
--------------

TTL_io_tensors_t holds two internal tensors ready for processing: imported_to as
input tensor and to_export_from as output tensor:

.. code-block:: c

    typedef struct {
    TTL_int_tensor_t imported_to;
    TTL_int_tensor_t to_export_from;
    } TTL_io_tensors_t;

    // Returns true if tensors are valid.
    // Useful when prolog/epilogs creates non-valid tensors.
    int TTL_tensors_empty(TTL_io_tensors_t tensors);