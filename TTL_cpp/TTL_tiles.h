/*
 * TTL_tiles.h
 *
 * Copyright (c) 2023 Mobileye
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "TTL_types.h"

/******************************************************
 * AUGMENTATION
 *****************************************************/

/**
 * @brief Augment an input tensor with logical padding
 *
 * When an input tensor is imported using a tiler the resulting tensors
 * may have elements beyond the space of the original tensor. In terms of the
 * origin tensor the subtensor may for example start at offset(-1, -1, -1) in
 * this case the elements at (-1, -1, -1), (-1, 0, 0) etc. needed to be created
 * from a process of augmentation.
 *
 * Currently the only dynamic part of the augmentation is the size of the
 * augmentation the augmented values are simply hardcode.
 *
 * A TTL_augmented_dim_t is the number of elements to augment.
 */

typedef unsigned char TTL_augmented_dim_t;  ///< A number of a "augmented" elements in the unit of elements

/**
 * @brief 3D description of the augmented margins
 *
 * TTL_augmentation represents the number of elements that a tensor will be expanded in each dimension.
 *
 * For example if left = 1 then one column of elements will be added to a tensor thereby increasing its width by 1.
 *
 * The type used to hold the augmentation of tiles along all dimensions
 */
struct TTL_augmentation {
    /**
     * @brief Create a 3D Description of a Tile augmentation
     *
     * @see TTL_overlap for more information.
     *
     * @param left       ///< Left hand augmentation in elements
     * @param right      ///< Right hand augmentation in elements
     * @param top        ///< Top augmentation in elements
     * @param bottom     ///< Bottom augmentation in elements
     * @param front      ///< Front augmentation in elements
     * @param back     ///< Back augmentation in elements
     *
     * @return A TTL_augmentation describing in 3D the overlap requested.
     */
    TTL_augmentation(const TTL_augmented_dim_t left = 0, const TTL_augmented_dim_t right = 0,
                     const TTL_augmented_dim_t top = 0, const TTL_augmented_dim_t bottom = 0,
                     const TTL_augmented_dim_t front = 0, const TTL_augmented_dim_t back = 0)
        : left(left), right(right), top(top), bottom(bottom), front(front), back(back) {}

    TTL_augmented_dim_t left;    ///< Left hand augmentation in elements
    TTL_augmented_dim_t right;   ///< Right hand augmentation in elements
    TTL_augmented_dim_t top;     ///< Top augmentation in elements
    TTL_augmented_dim_t bottom;  ///< Bottom augmentation in elements
    TTL_augmented_dim_t front;   ///< Front augmentation in elements
    TTL_augmented_dim_t back;    ///< Back augmentation in elements
};

/***
 * @brief A Tile is described by its Shape and the offset from the beginning of
 * the Space
 *
 * The type used to hold a tile of a space having a given shape and offset from
 * the beginning of the space
 */
struct TTL_tile {
    /**
     * @brief Create an empty tile. Empty means it has all dimensions set to zero
     *
     * Most operations on an empty tile should turn into no-ops and so an empty tile
     * is the safest default state.
     */
    TTL_tile() : shape(), offset() {}

    /**
     * @brief Check if the tile passed is empty.
     *
     * Empty is defined as width of the shape being equal to 0.
     *
     * @param tile The tile to check the emptiness of.
     *
     * @return true if shape is empty
     * @return false if shape is not empty
     */
    bool empty() const {
        return shape.empty();
    }

    TTL_shape shape;    ///< @see TTL_shape
    TTL_offset offset;  ///< @see TTL_offset
};

/**
 * @brief TTL_tiler is the basic unit that describes how a tile is subdivided.
 *
 * The TTL_tiler type represents the tiling of a 3D space into 3D tiles with
 * operational overlap
 */
struct TTL_tiler {
    /**
     * @brief Return a TTL_tiler based on a shape, a tile, and an overlap
     *
     * @param tensor_shape The shape to be tiled
     * @param tile_shape The description of the tile that the shape will be sub-divided to.
     * @param overlap The overlap between tiles
     * @param augmentation The augomentation to apply at the edges durring import.
     *
     * Complete description of what not how here.
     *
     * @return A tiler that can produce a tile for any given index.
     */
    TTL_tiler(const TTL_shape tensor_shape, const TTL_shape tile_shape, const TTL_overlap overlap,
              const TTL_augmentation augmentation)
        : space(tensor_shape), tile(tile_shape), overlap(overlap), augmentation(augmentation) {
        const TTL_dim_t tiles_in_width =
            TTL_ceil_of_a_div_b(tensor_shape.width + augmentation.left + augmentation.right - overlap.width,
                                tile_shape.width - overlap.width);
        const TTL_dim_t tiles_in_height =
            TTL_ceil_of_a_div_b(tensor_shape.height + augmentation.top + augmentation.bottom - overlap.height,
                                tile_shape.height - overlap.height);
        const TTL_dim_t tiles_in_depth =
            TTL_ceil_of_a_div_b(tensor_shape.depth + augmentation.front + augmentation.back - overlap.depth,
                                tile_shape.depth - overlap.depth);

        const TTL_dim_t tiles_in_plane = tiles_in_width * tiles_in_height;
        const TTL_dim_t number_of_tiles = tiles_in_plane * tiles_in_depth;

        cache = { number_of_tiles, tiles_in_width, tiles_in_height, tiles_in_depth, tiles_in_plane };
    }

    // Simplify creation of non-overlap tiler
    TTL_tiler(const TTL_shape shape, const TTL_shape tile)
        : TTL_tiler(shape, tile, TTL_overlap(), TTL_augmentation()) {}

    TTL_shape space;                ///< Represents the space to be tiled such as an image
    TTL_shape tile;                 ///< All tiles will be of this shape, except for clamping at
                                    ///< the end of the space
    TTL_overlap overlap;            ///< When zeroes represent no overlap
    TTL_augmentation augmentation;  ///< The augmentation that the tile produces.

    /**
     * @brief Precomputed information to speed up later reuse
     */
    struct {
        TTL_dim_t number_of_tiles;  ///<
        TTL_dim_t tiles_in_width;   ///<
        TTL_dim_t tiles_in_height;  ///<
        TTL_dim_t tiles_in_depth;   ///<
        TTL_dim_t tiles_in_plane;   ///<
    } cache;

    /**
     * @brief Return the number of tiles that this tile can produce.
     *
     * @return int The number of tiles produced by the tiler.
     */
    int number_of_tiles() const {
        return cache.number_of_tiles;
    }

    /**
     * @brief Given a tile ID return true or false to indicate if the id is valid
     *
     * @param tile_id The ID to validate.
     *
     * @return int The number of tiles produced by the tiler.
     */
    inline int valid_tile_id(int tile_id) const {
        return ((tile_id >= 0) && (tile_id < (int)cache.number_of_tiles));
    }

    /**
     * @brief Return the ceil value of a/b i.e. ceil(a/b)
     *
     * Implementation of ceil(a/b) without requiring a library or floating-point.
     *
     * Internal TTL function not part of the API.
     *
     * @param a The dividend to use in the calculation
     * @param b The divisor to use in the calculation
     *
     * @todo b is a shape dimension of a tensor, so it is not zero. Remove dynamic
     * check for zero b, w/o crashing the compiler.
     */
    inline int TTL_ceil_of_a_div_b(const int a, const int b) const {
        return b ? ((a + b - 1) / b) : 0;
    }

    TTL_dim_t tiles_in_width() const {
        return cache.tiles_in_width;
    }

    TTL_dim_t tiles_in_height() const {
        return cache.tiles_in_height;
    }

    TTL_dim_t tiles_in_depth() const {
        return cache.tiles_in_depth;
    }

    /**
     * @brief Returns a tile at a position from a tiler and respective coordinates.
     *
     * @param x The x position of the tile being created
     * @param y The y position of the tile being created
     * @param z The z position of the tile being created
     * @param tiler The tiler from which the tiler can be calculated.
     *
     * @return The created TTL_tile type
     */
    TTL_tile create_tile(TTL_dim_t x, TTL_dim_t y, TTL_dim_t z) const {
        TTL_tile result;

        // Calculate the offset in 3D
        result.offset = TTL_offset((x * (tile.width - overlap.width)) - augmentation.left,
                                   (y * (tile.height - overlap.height)) - augmentation.top,
                                   (z * (tile.depth - overlap.depth)) - augmentation.front);

        // Set the tile shape, clamping at the end of each dimension
        result.shape = tile;

        if (x == cache.tiles_in_width - 1) result.shape.width = space.width - result.offset.x + augmentation.right;

        if (y == cache.tiles_in_height - 1) result.shape.height = space.height - result.offset.y + augmentation.bottom;

        if (z == cache.tiles_in_depth - 1) result.shape.depth = space.depth - result.offset.z + augmentation.back;

        return result;
    }

    /**
     * @brief Return the tile_id'th tile of a tile array in row-major order.
     *
     * Return the tile_id'th tile, starting from tile_id=0, in row-major order.
     * Returns an invalid tile if tile_id is not valid (not from [0,
     number_of_tiles))

     * @param tile_id The tile id to return - if out of bounds then an invalid tile
     is returned
     * @param tiler The containing with the shape and tiling information
     *
     * @return The tile that is represented by tile_id when interpreted in row-major
     order.
     */
    TTL_tile get_tile(const int tile_id) const {
        if (valid_tile_id(tile_id) == false) {
            TTL_tile invalid;
            return invalid;
        }

        // Compute the 3D coordinates of the tile in order to compute its offset
        const TTL_dim_t z = tile_id / cache.tiles_in_plane;
        const TTL_dim_t tid_in_plane = tile_id % cache.tiles_in_plane;
        const TTL_dim_t y = tid_in_plane / cache.tiles_in_width;
        const TTL_dim_t x = tid_in_plane % cache.tiles_in_width;

        return create_tile(x, y, z);
    }

    /**
     * @brief Return the tile_id'th tile of a tile array in column-major order.
     *
     * Return the tile_id'th tile, starting from tile_id=0, in column-major order.
     * Returns an invalid tile if tile_id is not valid (not from [0,
     number_of_tiles))

     * @param tile_id The tile id to return - if out of bounds then an invalid tile
     is returned
     * @param tiler The tiler containing the shape and tiling information
     *
     * @return The tile that is represented by tile_id when interpreted in
     column-major order.
     */
    TTL_tile get_tile_column_major(const int tile_id) const {
        // Compute the 3D coordinates of the tile in order to compute its offset
        const TTL_dim_t z = tile_id / cache.tiles_in_plane;
        const TTL_dim_t tid_in_plane = tile_id % cache.tiles_in_plane;
        const TTL_dim_t y = tid_in_plane % cache.tiles_in_height;
        const TTL_dim_t x = tid_in_plane / cache.tiles_in_height;

        return create_tile(x, y, z);
    }
};