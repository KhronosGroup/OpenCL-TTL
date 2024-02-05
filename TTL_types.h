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
typedef struct {
    TTL_dim_t width;   ///< Number of elements along dimension x.
    TTL_dim_t height;  ///< Number of rows along dimension y
    TTL_dim_t depth;   ///< Number of planes along dimension z
} TTL_shape_t;

/**
 * @brief Create a description of a Shape
 *
 * @see TTL_shape_t for more information.
 *
 * @param width The number of elements of the Tile Shape in the x-axis
 * @param height The number of elements of the Tile Shape in the y-axis
 * @param depth The number of elements of the Tile Shape in the z-axis
 *
 * @return A TTL_shape_t describing in Tile Shape requested.
 */
static inline TTL_shape_t __attribute__((overloadable))
TTL_create_shape(TTL_dim_t width, TTL_dim_t height, TTL_dim_t depth) {
    TTL_shape_t res = { width, height, depth };
    return res;
}

/**
 * @brief Create a 2D Description of a Tile Shape
 *
 * @see TTL_shape_t for more information.
 *
 * @param width The number of elements of the Tile Shape in the x-axis
 * @param height The number of elements of the Tile Shape in the y-axis
 *
 * depth defaults to 1
 *
 * @return A TTL_shape_t describing in Tile Shape requested.
 */
static inline TTL_shape_t __attribute__((overloadable)) TTL_create_shape(TTL_dim_t width, TTL_dim_t height) {
    return TTL_create_shape(width, height, 1);
}

/**
 * @brief Create a 1D Description of a Tile Shape
 *
 * @see TTL_shape_t for more information.
 *
 * @param width The number of elements of the Tile Shape in the x-axis
 *
 * height and depth default to 1
 *
 * @return A TTL_shape_t describing in Tile Shape requested.
 */
static inline TTL_shape_t __attribute__((overloadable)) TTL_create_shape(TTL_dim_t width) {
    return TTL_create_shape(width, 1, 1);
}

/**
 * @brief A Shape is empty if its width is 0
 */
static inline bool TTL_shape_empty(TTL_shape_t shape) {
    return shape.width == 0;
}

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
typedef struct {
    TTL_offset_dim_t x;  ///< Offset in dimension x.
    TTL_offset_dim_t y;  ///< Offset in dimension y.
    TTL_offset_dim_t z;  ///< Offset in dimension z.
} TTL_offset_t;

/**
 * @brief Returns a TTL_offset_t
 *
 * @param x The x offset
 * @param y The y offset
 * @param z The z offset
 *
 * @return x, y, z expressed as a TTL_offset_dim_t
 */
static inline __attribute__((overloadable)) TTL_offset_t TTL_create_offset(TTL_offset_dim_t x, TTL_offset_dim_t y,
                                                                           TTL_offset_dim_t z) {
    const TTL_offset_t result = { x, y, z };

    return result;
}

/**
 * @brief Returns a TTL_offset_t
 *
 * @param x The x offset
 * @param y The y offset
 *
 * @return x, y, z = 0 expressed as a TTL_offset_dim_t
 */
static inline __attribute__((overloadable)) TTL_offset_t TTL_create_offset(TTL_offset_dim_t x, TTL_offset_dim_t y) {
    return TTL_create_offset(x, y, 0);
}

/**
 * @brief Returns a TTL_offset_t
 *
 * @param x The x offset
 *
 * @return x, y = 0, z = 0 expressed as a TTL_offset_dim_t
 */
static inline __attribute__((overloadable)) TTL_offset_t TTL_create_offset(TTL_offset_dim_t x) {
    return TTL_create_offset(x, 0, 0);
}

/**
 * @brief Returns a TTL_offset_t
 *
 *
 * @return x = 0, y = 0, z = 0 expressed as a TTL_offset_dim_t
 */
static inline __attribute__((overloadable)) TTL_offset_t TTL_create_offset(__TTL_NO_PARAMETERS) {
    return TTL_create_offset(0, 0, 0);
}

/******************************************************
 * OVERLAP
 *****************************************************/

typedef unsigned char TTL_overlap_dim_t;  ///< Overlap of a "adjacent" tiles in
                                          ///< the unit of elements
/**
 * @brief Description of the overlap in 3D space of adjacent tiles.
 *
 * TTL_overlap_t represents the number of overlapped elements between
 * adjacent tiles in each dimension.
 *
 * For example, overlap.x=1 means that every horizontally-adjacent
 * tiles share elements on one column.
 *
 * The type used to hold the overlap between adjacent tiles along all dimensions
 */
typedef struct {
    TTL_overlap_dim_t width;   ///< width overlap in elements
    TTL_overlap_dim_t height;  ///< height overlap in elements
    TTL_overlap_dim_t depth;   ///< depth overlap in elements
} TTL_overlap_t;

/**
 * @brief Create a 3D Description of a Tile overlap
 *
 * @see TTL_overlap_t for more information.
 *
 * @param width   ///< Overlap width in elements
 * @param height   ///< Overlap height in elements
 * @param depth   ///< Overlap depth in elements
 *
 * @return A TTL_overlap_t describing in 3D the overlap requested.
 */
static inline TTL_overlap_t __attribute__((overloadable))
TTL_create_overlap(const TTL_overlap_dim_t width, const TTL_overlap_dim_t height, const TTL_overlap_dim_t depth) {
    const TTL_overlap_t res = { width, height, depth };
    return res;
}

/**
 * @brief Create a 2D Description of a Tile overlap
 *
 * @see TTL_overlap_t for more information.
 *
 * @param width   ///< Overlap width in elements
 * @param height   ///< Overlap height in elements
 *
 * depth defaults to 0
 *
 * @return A TTL_overlap_t describing in 3D the overlap requested.
 */
static inline TTL_overlap_t __attribute__((overloadable))
TTL_create_overlap(const TTL_overlap_dim_t width, const TTL_overlap_dim_t height) {
    return TTL_create_overlap(width, height, 0);
}

/**
 * @brief Create a 1D Description of a Tile overlap
 *
 * @see TTL_overlap_t for more information.
 *
 * @param width   ///< Overlap width in elements
 *
 * height and depth default to 0
 *
 * @return A TTL_overlap_t describing in 3D the overlap requested.
 */
static inline TTL_overlap_t __attribute__((overloadable)) TTL_create_overlap(const TTL_overlap_dim_t width) {
    return TTL_create_overlap(width, 0, 0);
}

// TEMP WILL BE PROVIDED BY THE COMPILER
#define CLK_ASYNC_LINKED_LIST_NODE_SIZE 20

/**
 * @brief An async copy list node
 *
 * When performing gather type operations on async inputs, each gathered entity is
 * stored in a TTL_async_node_data
 *
 * The contents of the node are target specific, thes structure eases the
 * manipulation and usage of nodes
 */
typedef struct {
    unsigned char anonymous_data[CLK_ASYNC_LINKED_LIST_NODE_SIZE];
} TTL_async_node_data;

/**
 * @brief Map a index on the row to a physical index and height on the source
 */
typedef struct {
    TTL_offset_dim_t row_offset;  ///< The start point of the row
} TTL_row_gather_map_element;

/**
 * @brief Map a index on the row to a physical index and height on the source
 */
typedef struct {
    unsigned long ptr_elements;  ///< The elements in the map
    TTL_offset_dim_t index_offset;      ///< The index to apply when accessing TTL_row_gather_map_element[]
} TTL_row_gather_map;

static inline TTL_row_gather_map_element *TTL_get_elements(const TTL_row_gather_map row_gather_map) {
    return (TTL_row_gather_map_element *)row_gather_map.ptr_elements;
}

static inline TTL_row_gather_map TTL_empty_row_gather_map() {
    const TTL_row_gather_map result = {0, 0};

    return result;
}
