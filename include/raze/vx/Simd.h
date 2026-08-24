#pragma once 

#include <raze/vx/Mask.h>
#include <src/raze/vx/reference/SimdElementReference.h>

#include <raze/vx/Abi.h>
#include <src/raze/vx/Storage.h>
#include <raze/vx/Algorithm.h>


raze_disable_warning_msvc(26495)


__RAZE_VX_NAMESPACE_BEGIN

using aligned_policy    = __aligned_policy;
using unaligned_policy  = __unaligned_policy;

template <bool _Alignment_>
struct alignment_policy {
    static constexpr bool __alignment = _Alignment_;
};

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
    static constexpr auto __isa = Abi::isa;
    static constexpr auto __width = (Abi::size * sizeof(T) * 8);
    static constexpr auto __size = Abi::size;
    static constexpr auto __has_scalar_chunks = (Abi::size % 16) != 0;

    using storage_type  = vector_storage<T, Abi>;
    using reference     = _Simd_element_reference<simd>;
    using value_type    = T;
    using mask_type     = simd_mask<T, Abi>;
    using abi_type      = Abi;

    /**
     * @brief Constructs an uninitialized SIMD vector.
     *
     * The contents of the vector are unspecified. Use `fill()` or `broadcast()`
     * to initialize all lanes explicitly.
    */
    raze_no_stack_protector simd() noexcept = default;
    raze_no_stack_protector ~simd() noexcept = default;
    raze_no_stack_protector simd(const simd&) noexcept = default;
    raze_no_stack_protector simd(simd&&) noexcept = default;
    raze_no_stack_protector simd& operator=(const simd&) noexcept = default;
    raze_no_stack_protector simd& operator=(simd&&) noexcept = default;


    /**
     * @brief Constructs a SIMD vector by broadcasting a scalar value.
     *
     * @param value  The scalar value to broadcast into all lanes.
    */
    raze_no_stack_protector raze_always_inline explicit(false) simd(value_type v) noexcept {
        fill(v);
    }

    raze_no_stack_protector raze_always_inline simd(const storage_type& s) noexcept {
        _storage = s;
    }

    /**
     * @brief Returns a SIMD vector with all lanes set to zero.
    */
    raze_nodiscard static raze_no_stack_protector raze_always_inline simd zero() noexcept {
        simd r;

        r.__for_each_chunk([&] <class Chunk> (Chunk& chunk) raze_always_inline_lambda {
            using Storage = std::remove_cvref_t<decltype(ustorage(chunk))>;
            chunk = _Zero<__isa, Storage>()();
        });

        return r;
    }

    /**
     * @brief Returns a SIMD vector with all lanes set to `value`.
    */
    raze_nodiscard raze_no_stack_protector static raze_always_inline simd broadcast(value_type v) noexcept {
        simd r {};
        
        r.__for_each_chunk([&] <class Chunk> (_Chunk& chunk) raze_always_inline_lambda {
            using Storage = std::remove_cvref_t<decltype(ustorage(chunk))>;
            chunk = _Broadcast<__isa, Storage>()(v);
        });

        return r;
    }

    /**
     * @brief Fills all lanes with `value`.
     * @return Reference to `*this`.
    */
    raze_no_stack_protector raze_always_inline simd& fill(value_type v) noexcept {
        __for_each_chunk([&] <class Chunk> (_Chunk& chunk) raze_always_inline_lambda {
            using Storage = std::remove_cvref_t<decltype(ustorage(chunk))>;
            chunk = _Broadcast<__isa, Storage>()(v);
        });

        return *this;
    }

    /**
     * @brief Element-wise logical left shift.
     *
     * @param shift  Number of bits to shift.
     *
     * Semantics:
     * 
     *  Logical shift for all element types.
     *  If `shift >= bit_width(value_type)`, the result is zero.
    */
    raze_always_inline friend simd operator<<(const simd& x, u32 shift) noexcept {
        return bit_shl(x, shift);
    }

    /**
     * @brief Element-wise right shift.
     *
     * @param shift  Number of bits to shift.
     *
     * Semantics:
     * 
     *  Unsigned types: logical shift.
     * 
     *  Signed types: arithmetic (sign-extending) shift.
     * 
     *  If `shift >= bit_width(value_type)`, the result is zero or
     *  sign-extended depending on type.
    */
    raze_always_inline friend simd operator>>(const simd& x, u32 shift) noexcept {
        return bit_shr(x, shift);
    }

    template <class L, class R>
    raze_always_inline friend simd operator-(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return sub(x, y);
    }

    template <class L, class R>
    raze_always_inline friend simd operator+(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return add(x, y);
    }

    template <class L, class R>
    raze_always_inline friend simd operator*(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return mul(x, y);
    }

    template <class L, class R>
    raze_always_inline friend simd operator/(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return div(x, y);
    }

    template <class L, class R>
    raze_always_inline friend simd operator&(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return bit_and(x, y);
    }

    template <class L, class R>
    raze_always_inline friend simd operator|(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return bit_or(x, y);
    }

    template <class L, class R>
    raze_always_inline friend simd operator^(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return bit_xor(x, y);
    }

    raze_always_inline simd operator~() const noexcept {
        return bit_not(*this);
    }

    template <class L, class R>
    raze_always_inline friend mask_type operator==(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_equal(x, y);
    }

    template <class L, class R>
    raze_always_inline friend mask_type operator!=(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_not_equal(x, y);
    }

    template <class L, class R>
    raze_always_inline friend mask_type operator<(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_less(x, y);
    }

    template <class L, class R>
    raze_always_inline friend mask_type operator<=(const L& x, const R& y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return is_less_equal(x, y);
    }

    template <class L, class R>
    raze_always_inline friend mask_type operator>(const L& __x, const R& __y) noexcept
        requires(correct_simd_binary_op<simd, L, R>)
    {
        return is_greater(x, y);
    }

    template <class L, class R>
    raze_always_inline friend mask_type operator>=(const L& __x, const R& __y) noexcept
        requires(correct_simd_binary_op<simd, L, R>) 
    {
        return is_greater_equal(x, y);
    }

    raze_always_inline simd& operator>>=(u32 shift) noexcept {
        return *this = (*this >> shift);
    }

    raze_always_inline simd& operator<<=(u32 shift) noexcept {
        return *this = (*this << shift);
    }

    raze_always_inline simd& operator&=(const simd& other) noexcept {
        return *this = (*this & other);
    }

    raze_always_inline simd& operator|=(const simd& other) noexcept {
        return *this = (*this | other);
    }

    raze_always_inline simd& operator^=(const simd& other) noexcept {
        return *this = (*this ^ other);
    }

    raze_always_inline simd& operator+=(const simd& other) noexcept {
        return *this = (*this + other);
    }

    raze_always_inline simd& operator-=(const simd& other) noexcept {
        return *this = (*this - other);
    }

    raze_always_inline simd& operator*=(const simd& other) noexcept {
        return *this = (*this * other);
    }

    raze_always_inline simd& operator/=(const simd& other) noexcept {
        return *this = (*this / other);
    }

    raze_always_inline simd operator+() const noexcept {
        return _storage;
    }

    raze_always_inline simd operator-() const noexcept {
        return neg(*this);
    }

    raze_nodiscard raze_always_inline simd operator++(int) noexcept {
        simd self = *this;
        *this += simd(1);
        return self;
    }

    raze_always_inline simd& operator++() noexcept {
        return *this += simd(1);
    }

    raze_always_inline simd operator--(int) noexcept {
        simd self = *this;
        *this -= simd(1);
        return self;
    }

    raze_always_inline simd& operator--() noexcept {
        return *this -= simd(1);
    }

    raze_nodiscard raze_always_inline T operator[](i32 i) const noexcept {
        return __extract(i);
    }

    raze_nodiscard raze_always_inline reference operator[](i32 i) noexcept {
        return reference(*this, i);
    }

    template <sizetype _I_>
    raze_nodiscard raze_always_inline reference operator[](std::integral_constant<sizetype, _I_>) noexcept {
        return reference(*this, _I_);
    }

    template <sizetype _I_>
    raze_nodiscard raze_always_inline T operator[](std::integral_constant<sizetype, _I_> i) const noexcept {
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

    raze_nodiscard static constexpr bool is_native() noexcept {
        return storage_type::is_native();
    }

    template <sizetype _I_>
    raze_always_inline auto& __get() noexcept {
        return raze::vx::__get<_I_>(_storage.storage());
    }

    template <sizetype _I_>
    raze_always_inline const auto __get() const noexcept {
        return raze::vx::__get<_I_>(_storage.storage());
    }

    static constexpr auto __chunks_count() noexcept {
        return storage_type::chunks_count();
    }
private:
    raze_always_inline void __insert(i32 __position, value_type __value) noexcept {
        _storage.__insert(__position, __value);
    }

    raze_nodiscard raze_always_inline T __extract(i32 __i) const noexcept {
        return _storage.__extract(__i);
    }

    template <sizetype _I_>
    raze_nodiscard raze_always_inline T __extract(std::integral_constant<sizetype, _I_> __i) const noexcept {
        return _storage.__extract(__i);
    }

    friend _Simd_element_reference<simd>;
    raze_no_unique_address storage_type _storage;
};

__RAZE_VX_NAMESPACE_END
