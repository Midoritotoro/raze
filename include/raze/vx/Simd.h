#pragma once 

#include <src/raze/vx/reference/SimdElementReference.h>
#include <raze/vx/Mask.h>
#include <raze/vx/Abi.h>

#include <src/raze/vx/Storage.h>
#include <src/raze/vx/hw/configurable/arithmetic/Add.h>
#include <src/raze/vx/hw/configurable/arithmetic/Sub.h>
#include <src/raze/vx/hw/configurable/arithmetic/Mul.h>
#include <src/raze/vx/hw/configurable/arithmetic/Div.h>
#include <src/raze/vx/hw/configurable/arithmetic/Negate.h>
#include <src/raze/vx/hw/configurable/bitwise/ShiftRight.h>
#include <src/raze/vx/hw/configurable/bitwise/ShiftLeft.h>
#include <src/raze/vx/hw/configurable/compare/IsEqual.h>
#include <src/raze/vx/hw/configurable/compare/IsGreater.h>
#include <src/raze/vx/hw/configurable/compare/IsGreaterEqual.h>
#include <src/raze/vx/hw/configurable/compare/IsLess.h>
#include <src/raze/vx/hw/configurable/compare/IsLessEqual.h>
#include <src/raze/vx/hw/configurable/compare/IsNotEqual.h>
#include <src/raze/vx/hw/configurable/construct/Broadcast.h>


raze_disable_warning_msvc(26495)


__RAZE_VX_NAMESPACE_BEGIN

template <class V, class L, class R>
concept correct_simd_binary_op = simd_type<V> &&
    (std::same_as<std::remove_cvref_t<L>, V> && (
        std::convertible_to<std::remove_cvref_t<R>, typename V::value_type> ||
        std::same_as<std::remove_cvref_t<R>, V>)) ||
    (std::same_as<std::remove_cvref_t<R>, V> && (
        std::convertible_to<std::remove_cvref_t<L>, typename V::value_type> ||
        std::same_as<std::remove_cvref_t<L>, V>));

/**
 * @class simd
 * @brief A fixed-size SIMD vector abstraction with ISA/ABI‑aware backend dispatch.
 *
 * The `simd` class provides a high-level, type-safe interface for performing
 * vectorized arithmetic, logical, comparison, and shift operations across
 * multiple hardware instruction sets (SSE2, SSE4.1, AVX2, AVX‑512, etc.).
 * It wraps architecture-specific intrinsic types and exposes a uniform API
 * that behaves consistently across all supported ABIs.
 *
 * ## Key Characteristics
 * - Represents a vector of `value_type` elements stored in a native SIMD register.
 * - All operations are performed element-wise.
 *
 * ## Template Parameters
 * @tparam T  The scalar element type (e.g., i32, f32, f64).
 * @tparam Abi   ABI descriptor specifying ISA and register width.
*/

template <class T, class Abi>
class simd {
    static_assert(traits::is_vector_type_supported_v<std::decay_t<T>>, "Unsupported element type. ");
public:
    static constexpr auto isa = Abi::isa;
    static constexpr auto width = (Abi::size * sizeof(T) * 8);
    static constexpr auto has_scalar_chunks = (Abi::size % 16) != 0;

    using storage_type  = vector_storage<T, Abi>;
    using reference     = simd_element_reference<simd>;
    using value_type    = T;
    using mask_type     = simd_mask<T, Abi>;
    using abi_type      = Abi;
 
    raze_no_stack_protector simd() noexcept = default;
    raze_no_stack_protector ~simd() noexcept = default;
    raze_no_stack_protector simd(const simd&) noexcept = default;
    raze_no_stack_protector simd(simd&&) noexcept = default;
    raze_no_stack_protector simd& operator=(const simd&) noexcept = default;
    raze_no_stack_protector simd& operator=(simd&&) noexcept = default;
    raze_no_stack_protector simd(uninitialized_tag) noexcept {}

    /**
     * @brief Constructs a SIMD vector by broadcasting a scalar value.
     *
     * @param value  The scalar value to broadcast into all lanes.
    */
    raze_no_stack_protector raze_always_inline explicit(false) simd(value_type v) noexcept {
        *this = broadcast<simd>(v);
    }

    raze_no_stack_protector raze_always_inline simd(const storage_type& s) noexcept {
        _storage = s;
    }

    /**
     * @brief Element-wise left shift.
     *
     * Shifts every lane left by `shift` bits: `out[i] = x[i] << shift`.
     *
     * Semantics:
     * - Logical shift for all element types.
     * - If `shift >= bit_width(value_type)`, every lane becomes zero.
     *
     * @param x      The vector to shift.
     * @param shift  Per-lane shift amount (uniform across all lanes).
     * @return A vector with every lane shifted left by `shift`.
     *
     * @code
     * V h = v << 4;
     * @endcode
    */
    raze_always_inline friend simd operator<<(const simd& x, u32 shift) noexcept {
        return bit_shl(x, shift);
    }

    /**
     * @brief Element-wise right shift.
     *
     * Shifts every lane right by `shift` bits: `out[i] = x[i] >> shift`.
     *
     * Semantics:
     * - Unsigned `value_type`: logical shift.
     * - Signed `value_type`: arithmetic shift.
     * - If `shift >= bit_width(value_type)`, the result is zero (unsigned) or
     *   sign-extended (signed).
     *
     * @param x      The vector to shift.
     * @param shift  Per-lane shift amount (uniform across all lanes).
     * @return A vector with every lane shifted right by `shift`.
     *
     * @code
     * V h = v >> 4;
     * @endcode
    */
    raze_always_inline friend simd operator>>(const simd& x, u32 shift) noexcept {
        return bit_shr(x, shift);
    }

    /**
     * @brief Element-wise subtraction.
     *
     * Equivalent to `raze::vx::sub(x, y)`. Operand constraints identical to
     * `operator+`.
     *
     * @param x  Left operand: `simd` or scalar convertible to `value_type`.
     * @param y  Right operand: `simd` or scalar convertible to `value_type`.
     * @return `r` with `r[i] = x[i] - y[i]` for every lane `i`.
     *
     * @code
     * V r = a - b;
     * V t = 2 - b;
     * @endcode
    */
    template <class L, class R>
    raze_always_inline friend simd operator-(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return sub(x, y);
    }

    /**
     * @brief Element-wise addition.
     *
     * Equivalent to `raze::vx::add(x, y)`. Accepts either another `simd` of the
     * same type, or a scalar convertible to `value_type` on one side (broadcast
     * to all lanes).
     *
     * @param x  Left operand: `simd` or scalar convertible to `value_type`.
     * @param y  Right operand: `simd` or scalar convertible to `value_type`.
     * @return `r` with `r[i] = x[i] + y[i]` for every lane `i`.
     *
     * @code
     * V r = a + b;
     * V s = a + 1;
     * @endcode
    */
    template <class L, class R>
    raze_always_inline friend simd operator+(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return add(x, y);
    }

    /**
     * @brief Element-wise multiplication.
     *
     * Equivalent to `raze::vx::mul(x, y)`. Operand constraints identical to
     * `operator+`.
     *
     * @param x  Left operand: `simd` or scalar convertible to `value_type`.
     * @param y  Right operand: `simd` or scalar convertible to `value_type`.
     * @return `r` with `r[i] = x[i] * y[i]` for every lane `i`.
     *
     * @code
     * V r = a * b;
     * @endcode
    */
    template <class L, class R>
    raze_always_inline friend simd operator*(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return mul(x, y);
    }

    /**
     * @brief Element-wise division.
     *
     * Equivalent to `raze::vx::div(x, y)`. Operand constraints identical to
     * `operator+`.
     *
     * Division-by-zero, NaN, and rounding follow the `value_type` semantics;
     * the operator does not define a special behavior.
     *
     * @param x  Left operand: `simd` or scalar convertible to `value_type`.
     * @param y  Right operand: `simd` or scalar convertible to `value_type`.
     * @return `r` with `r[i] = x[i] / y[i]` for every lane `i`.
     *
     * @code
     * V r = a / b;
     * @endcode
    */
    template <class L, class R>
    raze_always_inline friend simd operator/(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return div(x, y);
    }

    /**
     * @brief Element-wise bitwise AND.
     * Equivalent to `raze::vx::bit_and(x, y)`.
     * Operand constraints identical to `operator+`.
     * @return `r` with `r[i] = x[i] & y[i]`.
     * @code
     * V m = a & 0xFF;
     * @endcode
    */
    template <class L, class R>
    raze_always_inline friend simd operator&(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return bit_and(x, y);
    }

    /**
     * @brief Element-wise bitwise OR.
     * Equivalent to `raze::vx::bit_or(x, y)`.
     * Operand constraints identical to `operator+`.
     * @return `r` with `r[i] = x[i] | y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend simd operator|(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return bit_or(x, y);
    }

    /**
     * @brief Element-wise bitwise XOR.
     * Equivalent to `raze::vx::bit_xor(x, y)`.
     * Operand constraints identical to `operator+`.
     * @return `r` with `r[i] = x[i] ^ y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend simd operator^(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return bit_xor(x, y);
    }

    /**
     * @brief Element-wise bitwise NOT.
     * Equivalent to `raze::vx::bit_not(*this)`.
     * @return `r` with `r[i] = ~(*this)[i]`.
     * @code
     * V f = ~a;
     * @endcode
    */
    raze_always_inline simd operator~() const noexcept {
        return bit_not(*this);
    }

    /**
     * @brief Element-wise equality comparison.
     * Equivalent to `raze::vx::is_equal(x, y)`.
     * @return `mask_type` with lane `i` true iff `x[i] == y[i]`.
     * @note Returns a mask. Use `m[i]` for a per-lane bool, or pass `m` to a
     *       masked overload (e.g. `raze::vx::add[m](a, b)`).
     * @code
     * auto m = a == b;
     * @endcode
    */
    template <class L, class R>
    raze_always_inline friend mask_type operator==(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_equal(x, y);
    }

    /**
     * @brief Element-wise inequality comparison.
     * Equivalent to `raze::vx::is_not_equal(x, y)`.
     * @return `mask_type` with lane `i` true iff `x[i] != y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend mask_type operator!=(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_not_equal(x, y);
    }

    /**
     * @brief Element-wise less-than comparison.
     * Equivalent to `raze::vx::is_less(x, y)`.
     * @return `mask_type` with lane `i` true iff `x[i] < y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend mask_type operator<(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_less(x, y);
    }

    /**
     * @brief Element-wise less-or-equal comparison.
     * Equivalent to `raze::vx::is_less_equal(x, y)`.
     * @return `mask_type` with lane `i` true iff `x[i] <= y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend mask_type operator<=(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return is_less_equal(x, y);
    }

    /**
     * @brief Element-wise greater-than comparison.
     * Equivalent to `raze::vx::is_greater(x, y)`.
     * @return `mask_type` with lane `i` true iff `x[i] > y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend mask_type operator>(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return is_greater(x, y);
    }

    /**
     * @brief Element-wise greater-or-equal comparison.
     * Equivalent to `raze::vx::is_greater_equal(x, y)`.
     * @return `mask_type` with lane `i` true iff `x[i] >= y[i]`.
    */
    template <class L, class R>
    raze_always_inline friend mask_type operator>=(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_greater_equal(x, y);
    }

    /**
     * @brief In-place element-wise right shift.
     * Equivalent to `*this = *this >> shift`.
     * @param shift  Per-lane shift amount.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator>>=(u32 shift) noexcept {
        return *this = (*this >> shift);
    }

    /**
     * @brief In-place element-wise left shift.
     * Equivalent to `*this = *this << shift`.
     * @param shift  Per-lane shift amount.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator<<=(u32 shift) noexcept {
        return *this = (*this << shift);
    }

    /**
     * @brief In-place element-wise bitwise AND.
     * Equivalent to `*this = *this & other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator&=(const simd& other) noexcept {
        return *this = (*this & other);
    }

    /**
     * @brief In-place element-wise bitwise OR.
     * Equivalent to `*this = *this | other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator|=(const simd& other) noexcept {
        return *this = (*this | other);
    }

    /**
     * @brief In-place element-wise bitwise XOR.
     * Equivalent to `*this = *this ^ other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator^=(const simd& other) noexcept {
        return *this = (*this ^ other);
    }

    /**
     * @brief In-place element-wise addition.
     * Equivalent to `*this = *this + other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator+=(const simd& other) noexcept {
        return *this = (*this + other);
    }

    /**
     * @brief In-place element-wise subtraction.
     * Equivalent to `*this = *this - other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator-=(const simd& other) noexcept {
        return *this = (*this - other);
    }

    /**
     * @brief In-place element-wise multiplication.
     * Equivalent to `*this = *this * other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator*=(const simd& other) noexcept {
        return *this = (*this * other);
    }

    /**
     * @brief In-place element-wise division.
     * Equivalent to `*this = *this / other`.
     * @param other  `simd` of the same type.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator/=(const simd& other) noexcept {
        return *this = (*this / other);
    }

    /**
     * @brief Unary plus: returns a copy of `*this`.
     *
     * Provided for symmetry with unary `operator-`, so that generic code
     * writing `+v` compiles. No computation is performed.
     *
     * @return A copy of `*this`.
    */
    raze_always_inline simd operator+() const noexcept {
        return _storage;
    }

    /**
     * @brief Element-wise negation.
     *
     * Equivalent to `raze::vx::neg(*this)`.
     * @return `r` with `r[i] = -(*this)[i]` for every lane `i`.
     *
     * @code
     * V n = -v;
     * @endcode
    */
    raze_always_inline simd operator-() const noexcept {
        return neg(*this);
    }

    /**
     * @brief Postfix increment: returns a copy, then adds 1 to every lane.
     *
     * Equivalent to:
     * ```cpp
     * simd tmp = *this;
     * *this += simd(1);
     * return tmp;
     * ```
     *
     * @return A copy of `*this` before the increment.
    */
    raze_nodiscard raze_always_inline simd operator++(int) noexcept {
        simd self = *this;
        *this += simd(1);
        return self;
    }
    
    /**
     * @brief Prefix increment: adds 1 to every lane, returns `*this`.
     *
     * Equivalent to `*this += simd(1); return *this;`.
     *
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator++() noexcept {
        return *this += simd(1);
    }

    /**
     * @brief Postfix decrement: returns a copy, then subtracts 1 from every lane.
     *
     * Equivalent to:
     * ```cpp
     * simd tmp = *this;
     * *this -= simd(1);
     * return tmp;
     * ```
     *
     * @return A copy of `*this` before the decrement.
    */
    raze_always_inline simd operator--(int) noexcept {
        simd self = *this;
        *this -= simd(1);
        return self;
    }

    /**
     * @brief Prefix decrement: subtracts 1 from every lane, returns `*this`.
     *
     * Equivalent to `*this -= simd(1); return *this;`.
     *
     * @return Reference to `*this`.
    */
    raze_always_inline simd& operator--() noexcept {
        return *this -= simd(1);
    }

    /**
     * @brief Returns the value of lane `i` (runtime index, read-only).
     *
     * @param i  Lane index; must satisfy `0 <= i < size()`.
     * @return The scalar value stored in lane `i`.
    */
    raze_nodiscard raze_always_inline T operator[](i32 i) const noexcept {
        return __extract(i);
    }

    /**
     * @brief Returns a mutable proxy referring to lane `i` (runtime index).
     *
     * The proxy supports assignment (`v[i] = 42;`) and read-through. It is a
     * `simd_element_reference<simd>`, not a `T&`, but behaves like one for
     * scalar assignment/read.
     *
     * @param i  Lane index; must satisfy `0 <= i < size()`.
     * @return A proxy referring to lane `i` of `*this`.
    */
    raze_nodiscard raze_always_inline reference operator[](i32 i) noexcept {
        return reference(*this, i);
    }

    /**
     * @brief Returns a mutable proxy referring to lane `I` (compile-time index).
     *
     * @tparam I  Compile-time lane index; must satisfy `0 <= I < size()`.
     * @return A proxy referring to lane `I` of `*this`.
    */
    template <sizetype I>
    raze_nodiscard raze_always_inline reference operator[](std::integral_constant<sizetype, I>) noexcept {
        return reference(*this, I);
    }

    /**
     * @brief Returns the value of lane `I` (compile-time index, read-only).
     *
     * @tparam I  Compile-time lane index; must satisfy `0 <= I < size()`.
     * @return The scalar value stored in lane `I`.
    */
    template <sizetype I>
    raze_nodiscard raze_always_inline T operator[](std::integral_constant<sizetype, I> i) const noexcept {
        return __extract(i);
    }

    raze_nodiscard static raze_always_inline constexpr i32 size() noexcept {
        return abi_type::size;
    }

    raze_nodiscard static raze_always_inline constexpr i32 length() noexcept {
        return size();
    }

    raze_nodiscard explicit raze_always_inline operator storage_type() const noexcept {
        return _storage;
    }

    raze_nodiscard explicit raze_always_inline operator storage_type& () noexcept {
        return _storage;
    }

    raze_nodiscard static constexpr bool is_native() noexcept {
        return storage_type::is_native();
    }

    raze_nodiscard raze_always_inline storage_type __storage() const noexcept {
        return _storage;
    }

    raze_nodiscard raze_always_inline storage_type& __storage() noexcept {
        return _storage;
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
        _storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
        _storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <sizetype I>
    raze_always_inline auto& __get() noexcept {
        return raze::vx::get<I>(_storage.storage());
    }

    template <sizetype I>
    raze_always_inline const auto __get() const noexcept {
        return raze::vx::get<I>(_storage.storage());
    }

    static constexpr auto __chunks_count() noexcept {
        return storage_type::chunks_count();
    }
private:
    raze_always_inline void __insert(i32 position, value_type value) noexcept {
        _storage.insert(position, value);
    }

    raze_nodiscard raze_always_inline T __extract(i32 i) const noexcept {
        return _storage.extract(i);
    }

    template <sizetype I>
    raze_nodiscard raze_always_inline T __extract(std::integral_constant<sizetype, I> i) const noexcept {
        return _storage.extract(i);
    }

    friend simd_element_reference<simd>;
    raze_no_unique_address storage_type _storage;
};

__RAZE_VX_NAMESPACE_END
