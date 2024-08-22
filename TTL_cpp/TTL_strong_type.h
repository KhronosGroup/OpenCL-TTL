/*
 * TTL_strong_type.h
 *
 * Copyright (c) 2025 Mobileye
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

#include <stdint.h>

/**
 * @brief Wrapper for strong type.
 *
 * Strong types reinforce interfaces by making them more expressive,
 * and less error-prone by forcing the right type.
 * This is implemented by the following thin wrapper.
 *
 * For example, In order to define a FrequencyKHz, uint16_t type:
 *
 * enum class StKHz : uint32_t;   				// This just needs to be a unique.
 * using KHz = TTL_StrongType<uint16_t, StKHz>;
 *
 * StrongTypes can be used in anywhere because to the outside world it looks exactly
 * like the underlying type.
 *
 * So a structure from the outside world can contain the follow when a KHz frequency is
 * expected as a uint16_t;
 *  *
 * struct ExternalData {
 *    KHz input_frequency_khz;
 * }
 *
 * would be 100% compability with the other unsafe code in the external entity that
 * simple said
 *
 * struct ExternalData {
 *    uint16_t input_frequency_khz;
 * }
 */
template <typename T, typename UNIQUE_ENUM_CLASS>
struct TTL_StrongType {
    typedef T TYPE;

    /* Default constructor which initializes members with default (calling the default constructor). */
    __attribute__((always_inline)) constexpr TTL_StrongType() : value() {}

    /* Construction from a fundamental value. */
    __attribute__((always_inline)) constexpr TTL_StrongType(T value) : value(value) {}

    /* Conversion back to the fundamental data type. */
    __attribute__((always_inline)) constexpr T Underlying() const {
        return value;
    }

    /**
     * It is acceptable to negate the underlying type
     *
     * - -2 Cats = 2 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator-() const {
        return TTL_StrongType(-value);
    }

    /**
     * It is acceptable to add 2 things of the same type. The rules of the underlying
     * value are used for the addition.
     *
     * 2 Cats + 2 Cats = 4 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType &operator+=(TTL_StrongType const &rhs) {
        value += rhs.value;
        return *this;
    }

    /**
     * It is acceptable to add 2 things of the same type. The rules of the underlying
     * value are used for the addition.
     *
     * 2 Cats + 2 Cats = 4 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator+(TTL_StrongType const &rhs) const {
        return TTL_StrongType(value) += rhs;
    }

    /**
     * Prefix increment
     *
     * It is acceptable to add another thing of the same type. The rules of the underlying
     * value are used for the addition.
     *
     * 2 Cats + 1 Cats = 3 Cats.
     */
    __attribute__((always_inline)) TTL_StrongType &operator++() {
        ++value;
        return *this;
    }

    /**
     * Postfix increment
     *
     * It is acceptable to add another thing of the same type. The rules of the underlying
     * value are used for the addition.
     *
     * 2 Cats + 1 Cats = 3 Cats.
     */
    __attribute__((always_inline)) TTL_StrongType operator++(int) {
        const TTL_StrongType current_value = *this;
        ++*this;
        return current_value;
    }

    /**
     * Prefix increment
     *
     * It is acceptable to subtract another thing of the same type. The rules of the underlying
     * value are used for the subtraction.
     *
     * 2 Cats - 1 Cats = 1 Cats.
     */
    __attribute__((always_inline)) TTL_StrongType &operator--() {
        --value;
        return *this;
    }

    /**
     * Postfix increment
     *
     * It is acceptable to subtract another thing of the same type. The rules of the underlying
     * value are used for the subtraction.
     *
     * 2 Cats - 1 Cats = 1 Cats.
     */
    __attribute__((always_inline)) TTL_StrongType operator--(int) {
        const TTL_StrongType current_value = *this;
        --*this;
        return current_value;
    }

    /**
     * It is acceptable to subtract 2 things of the same type. The rules of the underlying
     * value are used for the addition.
     *
     * 2 Cats - 2 Cats = 0 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType &operator-=(TTL_StrongType const &rhs) {
        value + rhs.value;
        return *this;
    }

    /**
     * It is acceptable to subtract 2 things of the same type. The rules of the underlying
     * value are used for the addition.
     *
     * 2 Cats - 2 Cats = 0 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator-(TTL_StrongType const &rhs) const {
        return TTL_StrongType(value) -= rhs;
    }

    /**
     * It is generally acceptable to multiple by the underlying type. The underlying type
     * tends to mean that sign rules are obeyed.
     *
     * 2 Cats * 2 = 4 Cats.
     * -2 Cats * -2 = 4 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator*(T rhs) const {
        return TTL_StrongType(value * rhs);
    }

    /**
     * It is generally acceptable to divide by the underlying type. The underlying type
     * tends to mean that sign rules are obeyed.
     *
     * 2 Cats / 2 = 1 Cats.
     * -2 Cats / -2 = 1 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator/(T rhs) const {
        return TTL_StrongType(value / rhs);
    }

    /**
     * It is generally acceptable to divide by the type.
     *
     * 2 Cats / 2 Cats = 1.
     * -2 Cats / 2 Cats = -1.
     */
    __attribute__((always_inline)) constexpr T operator/(TTL_StrongType rhs) const {
        return value / rhs.value;
    }

    /**
     * It is generally acceptable to modulo by the type.
     *
     * 3 Cats % 2 Cats = 1 Cats.
     * -3 Cats % -2 = 1 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator%(TTL_StrongType rhs) const {
        return TTL_StrongType(value % rhs.value);
    }

    /**
     * It is generally acceptable to modulo by the underlying type. The underlying type
     * tends to mean that sign rules are obeyed.
     *
     * 3 Cats % 2 = 1 Cats.
     * -3 Cats % -2 = 1 Cats.
     */
    __attribute__((always_inline)) constexpr TTL_StrongType operator%(T rhs) const {
        return TTL_StrongType(value % rhs);
    }

    /**
     * It is generally acceptable to compare.
     *
     * 3 Cats > 2 Cats = true.
     */
    __attribute__((always_inline)) constexpr bool operator>(TTL_StrongType const &rhs) const {
        return value > rhs.value;
    }

    /**
     * It is generally acceptable to compare.
     *
     * 3 Cats >= 3 Cats = true.
     */
    __attribute__((always_inline)) constexpr bool operator>=(TTL_StrongType const &rhs) const {
        return value >= rhs.value;
    }

    /**
     * It is generally acceptable to compare.
     *
     * 3 Cats < 2 Cats = false.
     */
    __attribute__((always_inline)) constexpr bool operator<(TTL_StrongType const &rhs) const {
        return value < rhs.value;
    }

    /**
     * It is generally acceptable to compare.
     *
     * 3 Cats <= 2 Cats = false.
     */
    __attribute__((always_inline)) constexpr bool operator<=(TTL_StrongType const &rhs) const {
        return value <= rhs.value;
    }

    /**
     * It is generally acceptable to compare.
     *
     * 3 Cats == 2 Cats = false.
     */
    __attribute__((always_inline)) constexpr bool operator==(TTL_StrongType const &rhs) const {
        return value == rhs.value;
    }

    /**
     * It is generally acceptable to compare.
     *
     * 3 Cats != 2 Cats = true.
     */
    __attribute__((always_inline)) constexpr bool operator!=(TTL_StrongType const &rhs) const {
        return value != rhs.value;
    }

    /**
     * @brief Allow compile time access to the underling type, this can be useful for template parameters etc.
     */
    using Type = T;

    template <typename U>
    friend U &operator<<(U &os, const TTL_StrongType ttl_string_type) {
        return os << ttl_string_type.value;
    }

private:
    /* The actual fundamental value. */
    T value;

    /**
     * This makes the type unique. It cannot be accessed, uses no storage but gives
     * the type a unique signature.
     */
    static constexpr UNIQUE_ENUM_CLASS unique = UNIQUE_ENUM_CLASS(0);
};

/**
 * @brief Wrapper to allow a single conversion to another type.
 *
 * For example
 *
 * Conversion from Hz to Mhz can be achieved by defining
 *
 * using FrequencyHzToMHz = StrongTypeConversion<FrequencyMHz, FrequencyHz, 1, 1000000>;
 *
 * Then
 *
 * FrequencyHz hz(1000000);
 * FrequencyHz mhz = HzToMHz(hz);
 *
 * Would give mhz = 1.
 */

template <typename TARGET_TYPE, typename SOURCE_TYPE, uint32_t CONVERSION_MULTIPLY, uint32_t CONVERSION_DIVISION>
struct StrongTypeConversion : public TARGET_TYPE {
    StrongTypeConversion(SOURCE_TYPE source_value)
        : TARGET_TYPE(CONVERSION_MULTIPLY == 1 ? source_value.Underlying() / CONVERSION_DIVISION
                                               : source_value.Underlying() * CONVERSION_MULTIPLY) {
        static_assert((CONVERSION_MULTIPLY == 1) || (CONVERSION_DIVISION == 1),
                      "StrongTypeConversion must have one of the conversion parameters set to 1");
    }

    operator TARGET_TYPE() const {
        return TARGET_TYPE::Underlying();
    }
};
