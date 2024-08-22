/*
 * TTL_tensors_common.h
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

/************************************************************************************************************
 * Allow for sizeof anything - deals with sizeof(void)
 ************************************************************************************************************/

/**
 * @def TTL_SIZEOF
 *
 * @brief opencl doesn't like sizeof(void) so for it to be 1 like normal c
 *
 * OpenCl will produce error: invalid application of 'sizeof' to a void type
 *
 * This will need to be expanded with more types - or a different language used!
 */
#define TTL_SIZEOF(type) sizeof(type)

/************************************************************************************************************
 * Define Layout
 ***********************************************************************************************************/

/**
 * @brief Description of a Tensor layout in memory
 *
 * Each logical tensor is embedded in both global and local memories within some
 * enclosing physical tensors.
 *
 * This embedding is referred to as 'layout', which specifies the actual distance in elements
 * between the start of consecutive data elements in each dimension.
 *
 * For the first axis the distance is always 1 element and and so this value is not stored.
 *
 * line_length and plane_area in memory, in units of an element.
 */
struct TTL_layout {
    /**
     * @brief Create a 3D Description of a Tensor layout in memory
     *
     * @see TTL_layout for more information.
     *
     * @param row_spacing;   The distance between the start of consequtive rows in units of elements.
     * @param plane_spacing;   The distance between the start of consequtive planes in units of elements.
     *
     */
    TTL_layout(const TTL_dim_t row_spacing = 0, const TTL_dim_t plane_spacing = 0)
        : row_spacing(row_spacing), plane_spacing(plane_spacing) {}

    TTL_dim_t row_spacing;    ///< The distance between the start of consequtive rows in units of elements.
    TTL_dim_t plane_spacing;  ///< The distance between the start of consequtive planes in units of elements.
};

/**
 * @brief Calculate the absolute linear offset in elements, based on a given
 * tensor offset and layout
 *
 * @param offset The 3D offset of the required linear offset.
 * @param layout The layout of the offset being calculated
 *
 * @return The offset in linear address space of the 3D offset
 */
static inline TTL_offset_dim_t TTL_linearize(const TTL_offset &offset, const TTL_layout &layout) {
    return ((offset.z * layout.plane_spacing) + (offset.y * layout.row_spacing) + offset.x);
}

/************************************************************************************************************
 * Structure of a tensor
 ***********************************************************************************************************/

/**
 * @def __TTL_typedef_tensor_t
 *
 * @brief A poor mans base class for an a tensor in the passed address space
 *
 * TTL_int_tensor_base_t contains both the logical dimensions of a tile as well as
 * its physical mapping to memory.
 *
 * @param TTL_scope The scope of the creation can be TTL_global or TTL_local
 * @param const_1 The const name to place after the prefix - should be empty or const_
 * @param location The location of the tensor - should be ext or int
 * @param type The type of the tensor - should be any valid c type
 * @param const_2 The const type to create - should be empty or const
 */
template <typename TENSORTYPE>
struct TTL_tensor {
    /**
     * @brief __TTL_tensor_name(TTL_create_, const_2, location, type, sub, _5_params)
     *
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory
     * @param shape Description of the shape of the tensor that base points
     * @param layout The layout of the ## location ## tensor
     * @param offset The offset of the tensor from the base. @see TTL_offset
     * @param elem_size The size of a single element in the
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_tensor(TENSORTYPE *base, const TTL_shape &shape, const TTL_layout &layout, const TTL_offset &offset,
               const TTL_dim_t elem_size)
        : base(base + TTL_linearize(offset, layout)), elem_size(elem_size), layout(layout), shape(shape) {}

    /**
     * @brief Create an empty location tensor. Empty means it has all dimensions set
     * to zero
     *
     * Most operations on an empty tensor should turn into no-ops and so an empty
     * tensor is the safest default state.
     */
    TTL_tensor() : TTL_tensor(0, TTL_shape(), TTL_layout(), TTL_offset(), 0){};

    /**
     * @brief Create an empty location tensor. Empty means it has all dimensions set
     * to zero
     *
     * Most operations on an empty tensor should turn into no-ops and so an empty
     * tensor is the safest default state.
     */
    // TTL_tensor(const TTL_tensor &tensor)
    //     : TTL_tensor(tensor.base, tensor.shape, tensor.layout, TTL_offset(), tensor.elem_size){};

    /**
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )
     *
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory
     * @param shape Description of the shape of the tensor that base points
     * @param layout The layout of the ## location ## tensor
     * @param elem_size The size of a single element in the tensor
     *
     * @details
     * The layout of the created tensor will have an offset of (0, 0, 0)
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_tensor(TENSORTYPE *const base, const TTL_shape &shape, const TTL_layout &layout, const TTL_dim_t elem_size)
        : TTL_tensor(base, shape, layout, TTL_offset(), elem_size) {}

    /**
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )
     *
     * @param base A pointer to a global address
     * @param shape Description of the shape of the tensor that base points
     * @param layout The layout of the ## location ## tensor
     *
     * @details
     * The layout of the created tensor will have an offset of (0, 0, 0)\n
     * The element size is inferred from the base_address pointer type.\n
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_tensor(TENSORTYPE *const base, const TTL_shape &shape, const TTL_layout &layout)
        : TTL_tensor(base, shape, layout, TTL_offset(), TTL_SIZEOF(*base)) {}

    /**
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )
     *
     * @param base A pointer to a global address
     * @param shape Description of the shape of the tensor that base points to
     * @param elem_size The size of a single element in the tensor
     *
     * @details
     * Layout is inferred from the shape.
     * Offset is taken to be zero.
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_tensor(TENSORTYPE *const base, const TTL_shape &shape, const TTL_dim_t elem_size)
        : TTL_tensor(base, shape, TTL_layout(shape.width, shape.height), TTL_offset(), elem_size) {}

    /**
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )
     *
     * @param base A pointer to a global address
     * @param shape Description of the shape of the tensor that base points to
     *
     * @details
     * Element size is inferred from the base_address pointer type.
     * Layout is inferred from the shape.
     * Offset is taken to be zero.
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_tensor(TENSORTYPE *const base, const TTL_shape &shape)
        : TTL_tensor(base, shape, TTL_layout(shape.width, shape.height), TTL_offset(), TTL_SIZEOF(*base)) {}

    /**
     * @brief Cast a  __TTL_tensor_overloaded_name(_, const_1, location, type, , ) to a __TTL_tensor_name(TTL_,
     * const_1, location, type, , _t)
     *
     * This is a safe cast, and implimented as a function with helper macro for type safety
     *
     * @param tensor The __TTL_tensor_name(TTL_, , location, type, , _t) to be cast to a
     *               __TTL_tensor_name(TTL_, const_1, location, type, , _t)
     *
     * @return A __TTL_tensor_name(TTL_, const_1, location, type, , _t) version of the input tensor
     */
    operator TTL_tensor<const TENSORTYPE>() const {
        return TTL_tensor<const TENSORTYPE>(base, shape, layout, TTL_offset(), elem_size);
    }

    // /**
    //  * @brief Cast a __TTL_tensor_name(TTL_, const_ , location, type, , _t) *to a __TTL_tensor_name(TTL_, const_,
    //  * location, void, , _t) *
    //  *
    //  * Lower level function take a void * tensor, this cast allows any tensor to be easily cast to a void *
    //  *
    //  * This cast does remove information althought is nominally safe. It should be used sparingly.
    //  *
    //  * @param tensor The __TTL_tensor_name(TTL_, , location, type, , _t) to be cast to a
    //  *               __TTL_tensor_name(TTL_, const_1, location, type, , _t)
    //  *
    //  * @return A __TTL_tensor_name(TTL_, const_1, location, type, , _t) version of the input tensor
    //  */
    // static inline const __TTL_tensor_name(TTL_, const_1, location, void, , _t) *
    //     __attribute__((overloadable))
    //     TTL_to_void_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, , _t) * tensor) {
    //     return (const __TTL_tensor_name(TTL_, const_1, location, void, , _t) *)tensor;
    // }

    /**
     * @brief  Read a value from a tensor
     *
     * @param x The offset in the x dimension
     * @param y The offset in the y dimension
     * @param z The offset in the z dimension
     *
     * No bounds checking is performed.
     *
     * @return The value read
     */
    const TENSORTYPE &read(const unsigned int x, const unsigned int y = 0, const unsigned int z = 0) const {
        return base[x + (layout.row_spacing * y) + (layout.plane_spacing * z)];
    }

    /**
     * @brief  Write a value to a tensor
     *
     * @param value The value to write
     * @param x The offset in the x dimension
     * @param y The offset in the y dimension
     * @param z The offset in the z dimension
     */
    TENSORTYPE write(const TENSORTYPE value, const unsigned int x, const unsigned int y = 0, const unsigned int z = 0) {
        base[x + (layout.row_spacing * y) + (layout.plane_spacing * z)] = value;
        return value;
    }

    /**
     * @brief A Tensor is empty if its shape is empty @see TTL_shape_empty
     *
     * @param tensor The tensor to test for emptiness
     *
     * @return true is the tensor is empty
     * @return false is the tensor is not empty
     */
    bool empty() {
        return shape.empty();
    }

    TENSORTYPE *base;    /*!< The base address of the tensor in the local address space */
    TTL_dim_t elem_size; /*!< The sizeof the elements in the tensor */
    TTL_layout layout;   /*!< The layout of the tensor, @see TTL_layout */
    TTL_shape shape;     /*!< The shape of the tensor in 3 dimensions */
};

/**
 * @def __TTL_typedef_sub_tensor_t
 *
 * @brief A tensor plus its reference to its parent tensor
 *
 * __TTL_sub_tensor_t contains both the logical dimensions of a tile as well as
 * its physical mapping to memory.
 *
 * @param TTL_scope The scope of the creation can be TTL_global or TTL_local
 * @param const_1 The const name to place after the prefix - should be empty or const_
 * @param location The location of the tensor - should be ext or int
 * @param type The type of the tensor - should be any valid c type
 * @param const_2 The const type to create - should be empty or const
 */
template <typename TENSORTYPE>
struct TTL_sub_tensor {
    /**
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _7_params)
     *
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory
     * @param shape Description of the shape of the tensor that base points
     * @param layout The layout of the ## location ## tensor
     * @param elem_size The size of a single element in the
     * @param offset The offset of the tensor from the base. @see TTL_offset
     * @param origin_shape The shape of the tensor that originated the sub tensor
     * @param origin_offset The offset of the tensor from the souce tensor. @see TTL_offset
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_sub_tensor(TENSORTYPE *base, const TTL_shape &shape, const TTL_layout &layout, const TTL_dim_t elem_size,
                   const TTL_offset offset, const TTL_shape origin_shape, TTL_offset origin_offset)
        : tensor(base, shape, layout, offset, elem_size), origin(origin_shape, origin_offset) {}

    /**
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _5_params)
     *
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory
     * @param shape Description of the shape of the tensor that base points
     * @param layout The layout of the ## location ## tensor
     * @param origin_tensor The tensor that originated the sub tensor
     * @param sub_offset The offset of the tensor from the souce tensor. @see TTL_offset
     *
     * @details
     * The element size of the tensor is taken from the origin tensor
     * The offset of the sub tensor is taken to be (0, 0, 0)
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_sub_tensor(TENSORTYPE *const base, const TTL_shape &shape, const TTL_layout &layout,
                   const TTL_tensor<TENSORTYPE> &origin_tensor, const TTL_offset &sub_offset)
        : TTL_sub_tensor(base, shape, layout, origin_tensor.elem_size, TTL_offset(), origin_tensor.shape, sub_offset) {}

    /**
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _2_params)
     *
     * Simply create a sub_tensor from an origin tensor
     *
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory
     * @param origin_tensor The tensor that originated the sub tensor
     *
     * @details
     * The element size of the tensor is taken from the origin tensor
     * The offset of the sub tensor is taken to be (0, 0, 0)
     * The offset of the sub tensor relative to the source tensor is taken to be (0, 0, 0)
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_sub_tensor(TENSORTYPE const base, const TTL_tensor<TENSORTYPE> &origin_tensor)
        : TTL_sub_tensor(base, origin_tensor.shape, origin_tensor.layout, origin_tensor.elem_size, TTL_offset(),
                         origin_tensor.shape, TTL_offset()) {}

    /**
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _1_param)
     *
     * Simply create a sub_tensor from an origin tensor
     *
     * @param origin_tensor The tensor that originated the sub tensor
     *
     * @details
     * Effective creates a sub-tensor that is a tensor covering 100% of the source tensor
     *
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)
     */
    TTL_sub_tensor(const TTL_tensor<TENSORTYPE> &origin_tensor)
        : TTL_sub_tensor(origin_tensor.base, origin_tensor.shape, origin_tensor.layout, origin_tensor.elem_size,
                         TTL_offset(), origin_tensor.shape, TTL_offset()) {}

    /**
     * @brief Create an empty tiled internal tensor. Empty means it has all dimensions set
     * to zero
     *
     * Most operations on an empty tensor should turn into no-ops and so an empty
     * tensor is the safest default state.
     */
    TTL_sub_tensor() : TTL_sub_tensor(nullptr, TTL_shape(), TTL_layout(), 0, TTL_offset(), TTL_shape(), TTL_offset()) {}

    // /**
    //  * @brief Cast a __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) *const tensor) to a
    //  * __TTL_tensor_name(TTL_, const_, location, type, sub_, _t)
    //  *
    //  * This is a safe cast, and implimented as a function with helper macro for type safety
    //  *
    //  * @param tensor The __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) to be cast to a
    //  *               __TTL_tensor_name(TTL_, const, location, type, sub_, _t)
    //  *
    //  * @return A __TTL_tensor_name(TTL_, const_, location, type, sub_, _t) version of the input tensor
    //  */
    // static inline __TTL_tensor_name(TTL_, const_, location, type, sub_, _t) *
    //     __attribute__((overloadable))
    //         TTL_to_const_sub_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) *const tensor) {
    //     const void *const result = tensor;
    //     return (__TTL_tensor_name(TTL_, const_, location, type, sub_, _t) *)result;
    // }

    // /**
    //  * @brief Cast a __TTL_tensor_name(TTL_, const_ , location, type, sub_, _t) *to a __TTL_tensor_name(TTL_, const_,
    //  * location, void, sub_, _t) *
    //  *
    //  * Lower level function take a void * tensor, this cast allows any tensor to be easily cast to a void *
    //  *
    //  * This cast does remove information althought is nominally safe. It should be used sparingly.
    //  *
    //  * @param tensor The __TTL_tensor_name(TTL_, , location, type, , _t) to be cast to a
    //  *               __TTL_tensor_name(TTL_, const_1, location, type, , _t)
    //  *
    //  * @return A __TTL_tensor_name(TTL_, const_1, location, type, , _t) version of the input tensor
    //  */
    // static inline const __TTL_tensor_name(TTL_, const_1, location, void, sub_, _t) *
    //     __attribute__((overloadable))
    //         TTL_to_void_sub_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) * tensor) {
    //     return (const __TTL_tensor_name(TTL_, const_1, location, void, sub_, _t) *)tensor;
    // }

    /**
     * @brief  Read a value from a sub_tensor
     *
     * @param x The offset in the x dimension
     * @param y The offset in the y dimension
     * @param z The offset in the z dimension
     *
     * No bounds checking is performed.
     *
     * @return The value read
     */
    const TENSORTYPE &read(const unsigned int x, const unsigned int y = 0, const unsigned int z = 0) const {
        return tensor.read(x, y, z);
    }

    /**
     * @brief  Write a value to a sub_tensor
     *
     * @param value The value to write
     * @param x The offset in the x dimension
     * @param y The offset in the y dimension
     * @param z The offset in the z dimension
     */
    TENSORTYPE write(const TENSORTYPE value, const unsigned int x, const unsigned int y = 0, const unsigned int z = 0) {
        return tensor.write(value, x, y, z);
    }

    /**
     * @brief A Tensor is empty if its shape is empty @see TTL_shape_empty
     *
     * @param tensor The tensor to test for emptiness
     *
     * @return true is the tensor is empty
     * @return false is the tensor is not empty
     */
    bool empty() {
        return tensor.empty();
    }
    struct Origin {
        /**
         * @Brief Store of the origin information.
         *
         * @param shape The shape of the origin tensor in 3 dimensions
         * @param sub_offset The offset of the sub tensor from the origin sensor
         */
        Origin(TTL_shape shape, TTL_offset sub_offset) : shape(shape), sub_offset(sub_offset) {}

        TTL_shape shape;        ///< The shape of the origin tensor in 3 dimensions
        TTL_offset sub_offset;  ///< The offset of the sub tensor from the origin sensor
    };

    TTL_tensor<TENSORTYPE> tensor;
    Origin origin;
};
