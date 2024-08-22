/*
 * TTL_types.h
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

#include "TTL_macros.h"

/**
 * @brief Description of a Shape
 *
 * A Shape is a 3D description of an object.
 *
 * The units are elements
 */
struct TTL_shape {
    /**
     * @brief Create a description of a Shape
     *
     * @see TTL_shape for more information.
     *
     * @param width The number of elements of the Tile Shape in the x-axis
     * @param height The number of elements of the Tile Shape in the y-axis
     * @param depth The number of elements of the Tile Shape in the z-axis
     *
     * @return A TTL_shape describing in Tile Shape requested.
     */
    TTL_shape(TTL_dim_t width = 0, TTL_dim_t height = 1, TTL_dim_t depth = 1)
        : width(width), height(height), depth(depth) {}

    /**
     * @brief A Shape is empty if its width is 0
     */
    bool empty() const {
        return width == 0;
    }

    TTL_dim_t width;   ///< Number of elements along dimension x.
    TTL_dim_t height;  ///< Number of rows along dimension y
    TTL_dim_t depth;   ///< Number of planes along dimension z
};

/******************************************************
 * OFFSET
 *****************************************************/

/**
 * @brief Description of the 3D offset of an object.
 *
 * A offset of an object from some other reference point.
 *
 * The units are elements
 */
struct TTL_offset {
    /**
     * @brief Create a TTL_offset
     *
     * @param x The x offset
     * @param y The y offset
     * @param z The z offset
     */
    TTL_offset(TTL_offset_dim_t x = 0, TTL_offset_dim_t y = 0, TTL_offset_dim_t z = 0) : x(x), y(y), z(z) {}

    TTL_offset_dim_t x;  ///< Offset in dimension x.
    TTL_offset_dim_t y;  ///< Offset in dimension y.
    TTL_offset_dim_t z;  ///< Offset in dimension z.
};

/******************************************************
 * OVERLAP
 *****************************************************/

typedef unsigned char TTL_overlap_dim_t;  ///< Overlap of a "adjacent" tiles in
                                          ///< the unit of elements
/**
 * @brief Description of the overlap in 3D space of adjacent tiles.
 *
 * TTL_overlap represents the number of overlapped elements between
 * adjacent tiles in each dimension.
 *
 * For example, overlap.x=1 means that every horizontally-adjacent
 * tiles share elements on one column.
 *
 * The type used to hold the overlap between adjacent tiles along all dimensions
 */
struct TTL_overlap {
    /**
     * @brief Create a 3D Description of a Tile overlap
     *
     * @see TTL_overlap for more information.
     *
     * @param width   ///< Overlap width in elements
     * @param height   ///< Overlap height in elements
     * @param depth   ///< Overlap depth in elements
     *
     * @return A TTL_overlap describing in 3D the overlap requested.
     */
    TTL_overlap(const TTL_overlap_dim_t width = 0, const TTL_overlap_dim_t height = 0,
                const TTL_overlap_dim_t depth = 0)
        : width(width), height(height), depth(depth){};

    TTL_overlap_dim_t width;   ///< width overlap in elements
    TTL_overlap_dim_t height;  ///< height overlap in elements
    TTL_overlap_dim_t depth;   ///< depth overlap in elements
};
