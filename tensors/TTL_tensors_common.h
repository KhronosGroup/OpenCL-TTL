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

#include "../TTL_macros.h"
#include "../TTL_types.h"

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
#define TTL_SIZEOF(type) __TTL_STR_CONCAT1(sizeof_, type)

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
typedef struct {
    TTL_dim_t row_spacing;    ///< The distance between the start of consequtive rows in units of elements.
    TTL_dim_t plane_spacing;  ///< The distance between the start of consequtive planes in units of elements.
} TTL_layout_t;

/**
 * @brief Create a 3D Description of a Tensor layout in memory
 *
 * @see TTL_layout_t for more information.
 *
 * @param row_spacing;   The distance between the start of consequtive rows in units of elements.
 * @param plane_spacing;   The distance between the start of consequtive planes in units of elements.
 *
 * @return A TTL_layout_t describing in 3D the layout requested.
 */
static inline TTL_layout_t __attribute__((overloadable))
TTL_create_layout(const TTL_dim_t row_spacing, const TTL_dim_t plane_spacing) {
    const TTL_layout_t res = { row_spacing, plane_spacing };
    return res;
}

/**
 * @brief Create a 2D Description of a Tensor layout in memory
 *
 * @see TTL_layout_t for more information.
 *
 * @param row_spacing The distance between the start of consequtive rows in units of elements.
 *
 * The plane spacing is set to 0 - as no planes exist to be spaced.
 *
 * @return A TTL_layout_t describing in 3D the layout requested.
 */
static inline TTL_layout_t __attribute__((overloadable)) TTL_create_layout(const TTL_dim_t row_spacing) {
    return TTL_create_layout(row_spacing, 0);
}

/**
 * @brief Create a 1D Description of a Tensor layout in memory
 *
 * @see TTL_layout_t for more information.
 *
 * The row spacing is set to 0 - as no rows exist to be spaced.
 * The plane spacing is set to 0 - as no planes exist to be spaced.
 **
 * @return A TTL_layout_t describing in 3D the layout requested.
 */
static inline TTL_layout_t __attribute__((overloadable)) TTL_create_layout(__TTL_NO_PARAMETERS) {
    return TTL_create_layout(0, 0);
}

/**
 * @brief Calculate the absolute linear offset in elements, based on a given
 * tensor offset and layout
 *
 * @param offset The 3D offset of the required linear offset.
 * @param layout The layout of the offset being calculated
 *
 * @return The offset in linear address space of the 3D offset
 */
static inline TTL_offset_dim_t TTL_linearize(const TTL_offset_t offset, const TTL_layout_t layout) {
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
#define __TTL_typedef_tensor_t(TTL_scope, const_1, location, type, const_2)                                     \
    typedef struct {                                                                                            \
        TTL_scope(const_2 type *) base; /** @brief The base address of the tensor in the local address space */ \
        TTL_dim_t elem_size;            /** @brief The sizeof the elements in the tensor */                     \
        TTL_layout_t layout;            /** @brief The layout of the tensor, @see TTL_layout_t */               \
        TTL_shape_t shape;              /** @brief The shape of the tensor in 3 dimensions */                   \
    } __TTL_tensor_name(TTL_, const_1, location, type, , _t)

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
#define __TTL_typedef_sub_tensor_t(TTL_scope, const_1, location, type, const_2)                              \
    typedef struct {                                                                                         \
        /* This anonymous union allows the members of the tensor to be accessed directly using for example   \
         * 'tiled_tensor.base' but also the tensor information to be extracted like a downcast for the cases \
         * where a tensor is required 'tiled_tensor.tensor'.                                                 \
         * Effectively tiled_tensor.base and tiled_tensor.tensor.base are the same thing                     \
         * Not currently implemented because of problems with some compilers */                              \
        /* union {                                                                                           \
    TTL_ext_tensor_t(base_ptr_type) tensor;                                                                  \
    TTL_ext_tensor_base_t;                                                                                   \
    //};*/                                                                                                   \
        __TTL_tensor_name(TTL_, const_1, location, type, , _t) tensor;                                       \
        struct {                                                                                             \
            TTL_shape_t shape;       /** @brief The shape of the origin tensor in 3 dimensions */            \
            TTL_offset_t sub_offset; /** @brief The offset of the sub tensor from the origin sensor */       \
        } origin;                                                                                            \
    } __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t)

/************************************************************************************************************
 * Creation of tensors
 ************************************************************************************************************/

/**
 * @brief Implementation of TTL_create_int_tensor
 *
 * @see TTL_create_int_tensor for full API and parameter information
 *
 * @param TTL_scope The scope of the creation can be TTL_global or TTL_local
 * @param const_1 The const name to place after the prefix - should be empty or const_
 * @param location The location of the tensor - should be ext or int
 * @param type The type of the tensor - should be any valid c type
 * @param const_2 The const type to create - should be empty or const
 */
#define __TTL_create_tensor_impl(TTL_scope, const_1, location, type, const_2)                                     \
    /**                                                                                                           \
     * @brief __TTL_tensor_name(TTL_create_, const_2, location, type, sub, _5_params)                             \
     *                                                                                                            \
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory                        \
     * @param shape Description of the shape of the tensor that base points                                       \
     * @param layout The layout of the ## location ## tensor                                                      \
     * @param offset The offset of the tensor from the base. @see TTL_offset_t                                    \
     * @param elem_size The size of a single element in the                                                       \
     *                                                                                                            \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                                 \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, , _t) __attribute__((overloadable))            \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, , )(TTL_scope(const_2 type *) base,    \
                                                                               const TTL_shape_t shape,           \
                                                                               const TTL_layout_t layout,         \
                                                                               const TTL_offset_t offset,         \
                                                                               const TTL_dim_t elem_size) {       \
        const TTL_offset_dim_t offset_in_bytes = TTL_linearize(offset, layout) * elem_size;                       \
                                                                                                                  \
        __TTL_tensor_name(TTL_, const_1, location, type, , _t) result;                                            \
        result.base = (TTL_scope(const_2 type *))((TTL_scope(const_2 char *))base + offset_in_bytes);             \
        result.elem_size = elem_size;                                                                             \
        result.layout = layout;                                                                                   \
        result.shape = shape;                                                                                     \
                                                                                                                  \
        return result;                                                                                            \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief Cast a  __TTL_tensor_overloaded_name(_, const_1, location, type, , ) to a __TTL_tensor_name(TTL_,   \
     * const_1, location, type, , _t)                                                                             \
     *                                                                                                            \
     * This is a safe cast, and implimented as a function with helper macro for type safety                       \
     *                                                                                                            \
     * @param tensor The __TTL_tensor_name(TTL_, , location, type, , _t) to be cast to a                          \
     *               __TTL_tensor_name(TTL_, const_1, location, type, , _t)                                       \
     *                                                                                                            \
     * @return A __TTL_tensor_name(TTL_, const_1, location, type, , _t) version of the input tensor               \
     */                                                                                                           \
    static inline const __TTL_tensor_name(TTL_, const_, location, type, , _t) *                                   \
        __attribute__((overloadable))                                                                             \
            TTL_to_const_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, , _t) *const tensor) {     \
        const void *const result = tensor;                                                                        \
        return (const __TTL_tensor_name(TTL_, const_, location, type, , _t) *)result;                             \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief Cast a __TTL_tensor_name(TTL_, const_ , location, type, , _t) *to a __TTL_tensor_name(TTL_, const_, \
     * location, void, , _t) *                                                                                    \
     *                                                                                                            \
     * Lower level function take a void * tensor, this cast allows any tensor to be easily cast to a void *       \
     *                                                                                                            \
     * This cast does remove information althought is nominally safe. It should be used sparingly.                \
     *                                                                                                            \
     * @param tensor The __TTL_tensor_name(TTL_, , location, type, , _t) to be cast to a                          \
     *               __TTL_tensor_name(TTL_, const_1, location, type, , _t)                                       \
     *                                                                                                            \
     * @return A __TTL_tensor_name(TTL_, const_1, location, type, , _t) version of the input tensor               \
     */                                                                                                           \
    static inline const __TTL_tensor_name(TTL_, const_1, location, void, , _t) *                                  \
        __attribute__((overloadable))                                                                             \
            TTL_to_void_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, , _t) * tensor) {           \
        return (const __TTL_tensor_name(TTL_, const_1, location, void, , _t) *)tensor;                            \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief A Tensor is empty if its shape is empty @see TTL_shape_empty                                        \
     *                                                                                                            \
     * @param tensor The tensor to test for emptiness                                                             \
     *                                                                                                            \
     * @return true is the tensor is empty                                                                        \
     * @return false is the tensor is not empty                                                                   \
     */                                                                                                           \
    static inline bool __attribute__((overloadable)) __TTL_tensor_overloaded_name(                                \
        TTL_, const_1, location, type, , _empty)(__TTL_tensor_name(TTL_, const_1, location, type, , _t) tensor) { \
        return TTL_shape_empty(tensor.shape);                                                                     \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief Create an empty location tensor. Empty means it has all dimensions set                              \
     * to zero                                                                                                    \
     *                                                                                                            \
     * Most operations on an empty tensor should turn into no-ops and so an empty                                 \
     * tensor is the safest default state.                                                                        \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, , _t)                                          \
        __TTL_tensor_name(TTL_create_empty_, const_1, location, type, , )() {                                     \
        __TTL_tensor_name(TTL_, const_1, location, type, , _t)                                                    \
            result = { 0, 0, TTL_create_layout(), TTL_create_shape(0) };                                          \
                                                                                                                  \
        return result;                                                                                            \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief Create an empty location tensor. Empty means it has all dimensions set                              \
     * to zero                                                                                                    \
     *                                                                                                            \
     * @param unused Simply defined to allow selection of the functon to call.                                    \
     *                                                                                                            \
     * As above but a dummy parameter is used to select the version to all                                        \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, , _t) __attribute__((overloadable))            \
        __TTL_tensor_no_type_name(TTL_create_empty_, const_1, location, , )(TTL_scope(type *) unused) {           \
        (void)unused;                                                                                             \
        return __TTL_tensor_name(TTL_create_empty_, const_1, location, type, , )();                               \
    }

/**
 * @brief Implementation of TTL_create_int_sub_tensor
 *
 * @see TTL_create_int_sub_tensor for full API and parameter information
 */
#define __TTL_create_sub_tensor_impl(TTL_scope, const_1, location, type, const_2)                                     \
    /**                                                                                                               \
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _7_params)                                \
     *                                                                                                                \
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory                            \
     * @param shape Description of the shape of the tensor that base points                                           \
     * @param layout The layout of the ## location ## tensor                                                          \
     * @param elem_size The size of a single element in the                                                           \
     * @param offset The offset of the tensor from the base. @see TTL_offset_t                                        \
     * @param origin_shape The shape of the tensor that originated the sub tensor                                     \
     * @param origin_offset The offset of the tensor from the souce tensor. @see TTL_offset_t                         \
     *                                                                                                                \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                                     \
     */                                                                                                               \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) __attribute__((overloadable))            \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub_, )(TTL_scope(const_2 type *) base,    \
                                                                                   const TTL_shape_t shape,           \
                                                                                   const TTL_layout_t layout,         \
                                                                                   const TTL_dim_t elem_size,         \
                                                                                   const TTL_offset_t offset,         \
                                                                                   const TTL_shape_t origin_shape,    \
                                                                                   TTL_offset_t origin_offset) {      \
        __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) result;                                            \
                                                                                                                      \
        result.tensor = __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, , )(                       \
            base, shape, layout, offset, elem_size);                                                                  \
        result.origin.shape = origin_shape;                                                                           \
        result.origin.sub_offset = origin_offset;                                                                     \
                                                                                                                      \
        return result;                                                                                                \
    }                                                                                                                 \
                                                                                                                      \
    /**                                                                                                               \
     * @brief Cast a __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) *const tensor) to a                   \
     * __TTL_tensor_name(TTL_, const_, location, type, sub_, _t)                                                      \
     *                                                                                                                \
     * This is a safe cast, and implimented as a function with helper macro for type safety                           \
     *                                                                                                                \
     * @param tensor The __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) to be cast to a                   \
     *               __TTL_tensor_name(TTL_, const, location, type, sub_, _t)                                         \
     *                                                                                                                \
     * @return A __TTL_tensor_name(TTL_, const_, location, type, sub_, _t) version of the input tensor                \
     */                                                                                                               \
    static inline __TTL_tensor_name(TTL_, const_, location, type, sub_, _t) *                                         \
        __attribute__((overloadable))                                                                                 \
            TTL_to_const_sub_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) *const tensor) { \
        const void *const result = tensor;                                                                            \
        return (__TTL_tensor_name(TTL_, const_, location, type, sub_, _t) *)result;                                   \
    }                                                                                                                 \
                                                                                                                      \
    /**                                                                                                               \
     * @brief Cast a __TTL_tensor_name(TTL_, const_ , location, type, sub_, _t) *to a __TTL_tensor_name(TTL_, const_, \
     * location, void, sub_, _t) *                                                                                    \
     *                                                                                                                \
     * Lower level function take a void * tensor, this cast allows any tensor to be easily cast to a void *           \
     *                                                                                                                \
     * This cast does remove information althought is nominally safe. It should be used sparingly.                    \
     *                                                                                                                \
     * @param tensor The __TTL_tensor_name(TTL_, , location, type, , _t) to be cast to a                              \
     *               __TTL_tensor_name(TTL_, const_1, location, type, , _t)                                           \
     *                                                                                                                \
     * @return A __TTL_tensor_name(TTL_, const_1, location, type, , _t) version of the input tensor                   \
     */                                                                                                               \
    static inline const __TTL_tensor_name(TTL_, const_1, location, void, sub_, _t) *                                  \
        __attribute__((overloadable))                                                                                 \
            TTL_to_void_sub_tensor(const __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) * tensor) {       \
        return (const __TTL_tensor_name(TTL_, const_1, location, void, sub_, _t) *)tensor;                            \
    }                                                                                                                 \
                                                                                                                      \
    /**                                                                                                               \
     * @brief A Tensor is empty if its shape is empty @see TTL_shape_empty                                            \
     *                                                                                                                \
     * @param tensor The tensor to test for emptiness                                                                 \
     *                                                                                                                \
     * @return true is the tensor is empty                                                                            \
     * @return false is the tensor is not empty                                                                       \
     */                                                                                                               \
    static inline bool __attribute__((overloadable))                                                                  \
        __TTL_tensor_overloaded_name(TTL_, const_1, location, type, sub_, _empty)(                                    \
            __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) tensor) {                                      \
        return TTL_shape_empty(tensor.tensor.shape);                                                                  \
    }                                                                                                                 \
                                                                                                                      \
    /**                                                                                                               \
     * @brief Create an empty tiled internal tensor. Empty means it has all dimensions set                            \
     * to zero                                                                                                        \
     *                                                                                                                \
     * Most operations on an empty tensor should turn into no-ops and so an empty                                     \
     * tensor is the safest default state.                                                                            \
     */                                                                                                               \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t)                                          \
        __TTL_tensor_name(TTL_create_empty_, const_1, location, type, sub_, )() {                                     \
        __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) result;                                            \
                                                                                                                      \
        result.tensor = __TTL_tensor_name(TTL_create_empty_, const_1, location, type, , )();                          \
        result.origin.shape = TTL_create_shape(0);                                                                    \
        result.origin.sub_offset = TTL_create_offset();                                                               \
                                                                                                                      \
        return result;                                                                                                \
    }                                                                                                                 \
                                                                                                                      \
    /**                                                                                                               \
     * @brief Create an empty location sub tensor. Empty means it has all dimensions set                              \
     * to zero                                                                                                        \
     *                                                                                                                \
     * @param unused Simply defined to allow selection of the functon to call.                                        \
     *                                                                                                                \
     * As above but a dummy parameter is used to select the version to all                                            \
     */                                                                                                               \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub_, _t) __attribute__((overloadable))            \
        __TTL_tensor_no_type_name(TTL_create_empty_, const_1, location, sub_, )(TTL_scope(type *) unused) {           \
        (void)unused;                                                                                                 \
        return __TTL_tensor_name(TTL_create_empty_, const_1, location, type, sub_, )();                               \
    }

/************************************************************************************************************
 * Create the create tensor functions for different overloads
 ************************************************************************************************************/

#define __TTL_create_create_tensor_functions(TTL_scope, const_1, location, type, sub, const_2)                 \
    /**                                                                                                        \
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )                        \
     *                                                                                                         \
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory                     \
     * @param shape Description of the shape of the tensor that base points                                    \
     * @param layout The layout of the ## location ## tensor                                                   \
     * @param elem_size The size of a single element in the tensor                                             \
     *                                                                                                         \
     * @details                                                                                                \
     * The layout of the created tensor will have an offset of (0, 0, 0)                                       \
     *                                                                                                         \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                              \
     */                                                                                                        \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))      \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(TTL_scope(const_2 type *)    \
                                                                                      const base,              \
                                                                                  const TTL_shape_t shape,     \
                                                                                  const TTL_layout_t layout,   \
                                                                                  const TTL_dim_t elem_size) { \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                      \
            base, shape, layout, TTL_create_offset(), elem_size);                                              \
    }                                                                                                          \
                                                                                                               \
    /**                                                                                                        \
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )                        \
     *                                                                                                         \
     * @param base A pointer to a global address                                                               \
     * @param layout The layout of the ## location ## tensor                                                   \
     *                                                                                                         \
     * @details                                                                                                \
     * The layout of the created tensor will have an offset of (0, 0, 0)\n                                     \
     * The element size is inferred from the base_address pointer type.\n                                      \
     *                                                                                                         \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                              \
     */                                                                                                        \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))      \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                             \
            TTL_scope(const_2 type *) const base, const TTL_shape_t shape, const TTL_layout_t layout) {        \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                      \
            base, shape, layout, TTL_create_offset(), TTL_SIZEOF(type));                                       \
    }                                                                                                          \
                                                                                                               \
    /**                                                                                                        \
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )                        \
     *                                                                                                         \
     * @param base A pointer to a global address                                                               \
     * @param shape Description of the shape of the tensor that base points to                                 \
     * @param elem_size The size of a single element in the tensor                                             \
     *                                                                                                         \
     * @details                                                                                                \
     * Layout is inferred from the shape.                                                                      \
     * Offset is taken to be zero.                                                                             \
     *                                                                                                         \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                              \
     */                                                                                                        \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))      \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                             \
            TTL_scope(const_2 type *) const base, const TTL_shape_t shape, const TTL_dim_t elem_size) {        \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                      \
            base, shape, TTL_create_layout(shape.width, shape.height), TTL_create_offset(), elem_size);        \
    }                                                                                                          \
    /**                                                                                                        \
     * @brief __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )                        \
     *                                                                                                         \
     * @param base A pointer to a global address                                                               \
     * @param shape Description of the shape of the tensor that base points to                                 \
     *                                                                                                         \
     * @details                                                                                                \
     * Element size is inferred from the base_address pointer type.                                            \
     * Layout is inferred from the shape.                                                                      \
     * Offset is taken to be zero.                                                                             \
     *                                                                                                         \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                              \
     */                                                                                                        \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))      \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                             \
            TTL_scope(const_2 type *) const base, const TTL_shape_t shape) {                                   \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                      \
            base, shape, TTL_create_layout(shape.width, shape.height), TTL_create_offset(), TTL_SIZEOF(type)); \
    }

#define __TTL_create_create_sub_tensor_functions(TTL_scope, const_1, location, type, sub, const_2)                \
    /**                                                                                                           \
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _5_params)                            \
     *                                                                                                            \
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory                        \
     * @param shape Description of the shape of the tensor that base points                                       \
     * @param layout The layout of the ## location ## tensor                                                      \
     * @param offset The offset from the base to place the tensor                                                 \
     * @param origin_tensor The tensor that originated the sub tensor                                             \
     * @param sub_offset The offset of the tensor from the souce tensor. @see TTL_offset_t                        \
     *                                                                                                            \
     * @details                                                                                                   \
     * The element size of the tensor is taken from the origin tensor                                             \
     * The offset of the sub tensor is taken to be (0, 0, 0)                                                      \
     *                                                                                                            \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                                 \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))         \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                                \
            TTL_scope(const_2 type *) const base,                                                                 \
            const TTL_shape_t shape,                                                                              \
            const TTL_layout_t layout,                                                                            \
            const TTL_offset_t offset,                                                                            \
            const __TTL_tensor_name(TTL_, const_, ext_, type, , _t) origin_tensor,                                \
            const TTL_offset_t sub_offset) {                                                                      \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                         \
            base, shape, layout, origin_tensor.elem_size, offset, origin_tensor.shape, sub_offset);               \
    }                                                                                                             \
    /**                                                                                                           \
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _5_params)                            \
     *                                                                                                            \
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory                        \
     * @param shape Description of the shape of the tensor that base points                                       \
     * @param layout The layout of the ## location ## tensor                                                      \
     * @param origin_tensor The tensor that originated the sub tensor                                             \
     * @param sub_offset The offset of the tensor from the souce tensor. @see TTL_offset_t                        \
     *                                                                                                            \
     * @details                                                                                                   \
     * The element size of the tensor is taken from the origin tensor                                             \
     * The offset of the sub tensor is taken to be (0, 0, 0)                                                      \
     *                                                                                                            \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                                 \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))         \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                                \
            TTL_scope(const_2 type *) const base,                                                                 \
            const TTL_shape_t shape,                                                                              \
            const TTL_layout_t layout,                                                                            \
            const __TTL_tensor_name(TTL_, const_, ext_, type, , _t) origin_tensor,                                \
            const TTL_offset_t sub_offset) {                                                                      \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                         \
            base, shape, layout, origin_tensor.elem_size, TTL_create_offset(), origin_tensor.shape, sub_offset);  \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _2_params)                            \
     *                                                                                                            \
     * Simply create a sub_tensor from an origin tensor                                                           \
     *                                                                                                            \
     * @param base The ## TTL_scope ## base address of the tensor in ## location ## memory                        \
     * @param origin_tensor The tensor that originated the sub tensor                                             \
     *                                                                                                            \
     * @details                                                                                                   \
     * The element size of the tensor is taken from the origin tensor                                             \
     * The offset of the sub tensor is taken to be (0, 0, 0)                                                      \
     * The offset of the sub tensor relative to the source tensor is taken to be (0, 0, 0)                        \
     *                                                                                                            \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                                 \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))         \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                                \
            TTL_scope(const_2 type *) const base,                                                                 \
            const __TTL_tensor_name(TTL_, const_1, ext_, type, , _t) origin_tensor) {                             \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(base,                    \
                                                                                         origin_tensor.shape,     \
                                                                                         origin_tensor.layout,    \
                                                                                         origin_tensor.elem_size, \
                                                                                         TTL_create_offset(),     \
                                                                                         origin_tensor.shape,     \
                                                                                         TTL_create_offset());    \
    }                                                                                                             \
                                                                                                                  \
    /**                                                                                                           \
     * @brief  __TTL_tensor_name(TTL_create_, const_1, location, type, sub, _1_param)                             \
     *                                                                                                            \
     * Simply create a sub_tensor from an origin tensor                                                           \
     *                                                                                                            \
     * @param origin_tensor The tensor that originated the sub tensor                                             \
     *                                                                                                            \
     * @details                                                                                                   \
     * Effective creates a sub-tensor that is a tensor covering 100% of the source tensor                         \
     *                                                                                                            \
     * @return return a __TTL_tensor_name(TTL_, const_1, location, type, sub, _t)                                 \
     */                                                                                                           \
    static inline __TTL_tensor_name(TTL_, const_1, location, type, sub, _t) __attribute__((overloadable))         \
        __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(                                \
            const __TTL_tensor_name(TTL_, const_1, location, type, , _t) origin_tensor) {                         \
        return __TTL_tensor_overloaded_name(TTL_create_, const_1, location, type, sub, )(origin_tensor.base,      \
                                                                                         origin_tensor.shape,     \
                                                                                         origin_tensor.layout,    \
                                                                                         origin_tensor.elem_size, \
                                                                                         TTL_create_offset(),     \
                                                                                         origin_tensor.shape,     \
                                                                                         TTL_create_offset());    \
    }
