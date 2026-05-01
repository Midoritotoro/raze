#pragma once 

#include <src/raze/vx/hw/Compare.h>
#include <src/raze/vx/hw/Shuffle.h>

#include <src/raze/utility/Assert.h>
#include <src/raze/vx/hw/Access.h>

#include <raze/vx/Mask.h>
#include <raze/vx/SimdCast.h>
#include <src/raze/vx/hw/Memory.h>

#include <src/raze/vx/reference/SimdElementReference.h>
#include <src/raze/vx/reference/SimdBoolReference.h>
#include <src/raze/vx/reference/BitReference.h>

#include <raze/vx/Abi.h>
#include <src/raze/vx/Storage.h>
#include <raze/vx/Algorithm.h>


raze_disable_warning_msvc(26495)


__RAZE_VX_NAMESPACE_BEGIN

template <class _Iterator_>
concept any_iterator_or_pointer = std::input_or_output_iterator<_Iterator_> || std::is_pointer_v<_Iterator_>;

using aligned_policy    = __aligned_policy;
using unaligned_policy  = __unaligned_policy;

template <bool _Alignment_>
struct alignment_policy {
    static constexpr bool __alignment = _Alignment_;
};

struct aligned_mode {};
constexpr inline auto aligned = raze::options::flag(aligned_mode{});
struct aligned_option : raze::options::exact_option<aligned> {};

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
 * @tparam _Type_  The scalar element type (e.g., int32_t, float, double).
 * @tparam _Abi_   ABI descriptor specifying ISA and register width.
*/
template <
    class _Type_,
    class _Abi_>
class raze_aligned_type(64) simd {
    static_assert(type_traits::__is_vector_type_supported_v<std::decay_t<_Type_>>, "Unsupported element type. ");
public:
    static constexpr auto __isa = _Abi_::isa;
    static constexpr auto __width = (_Abi_::size * sizeof(_Type_) * 8);
    static constexpr auto __size = _Abi_::size;

    static constexpr std::size_t __bytes = _Abi_::size * sizeof(_Type_);
    static constexpr bool __use_native = (__bytes == 16 && __has_sse2_support_v<__isa>) || 
        (__bytes == 32 && __has_avx_support_v<__isa>) || (__bytes == 64 && __has_avx512f_support_v<__isa>);

    using storage_type  = std::conditional_t<__use_native, type_traits::__deduce_simd_vector_type<_Type_, __width>, _Simd_vector_tuple_type<_Type_, _Abi_>>;
    using reference     = _Simd_element_reference<simd>;
    using value_type    = _Type_;
    using mask_type     = simd_mask<_Type_, _Abi_>;
    using abi_type      = _Abi_;

    /**
     * @brief Indicates whether the current ISA/ABI supports native masked loads.
     *
     * This constant evaluates to `true` when the backend provides a hardware
     * implementation of masked loads for the given ISA, register width, and
     * element size. A “native” masked load means that the operation can be
     * performed directly using a dedicated instruction (e.g., AVX‑512
     * `vmaskload*`, `vpmaskmov*`) without requiring emulation.
     *
     * When `false`, masked loads are emulated using a combination of:
     * 
     * - an unmasked load, and
     * 
     * - a mask‑controlled blend operation.
     *
     * Native support is important for performance and for correctly handling
     * partial vectors (“tail elements”).
    */
    static constexpr inline bool is_native_mask_load_supported_v = __is_native_mask_load_supported_v<__isa, __width, value_type>;

    /**
     * @brief Indicates whether the current ISA/ABI supports native masked stores.
     *
     * This constant evaluates to `true` when the backend provides a hardware
     * instruction for masked stores for the given ISA, register width, and
     * element size. Native masked stores (e.g., AVX‑512 `vmaskstore*`,
     * `vpmaskmov*`) allow writing only selected lanes to memory without
     * additional blending or temporary buffers.
     *
     * When `false`, masked stores are emulated using:
     * 
     * - a load of the destination memory,
     * 
     * - a mask‑controlled blend with the source vector,
     * 
     * - a full unmasked store.
     *
     * Native support enables efficient and safe handling of tail elements,
     * avoids unnecessary memory traffic, and improves performance in
     * algorithms that rely on partial‑vector writes.
    */
    static constexpr inline bool is_native_mask_store_supported_v = __is_native_mask_store_supported_v<__isa, __width, value_type>;

    /**
     * @brief Constructs an uninitialized SIMD vector.
     *
     * The contents of the vector are unspecified. Use `fill()` or `broadcast()`
     * to initialize all lanes explicitly.
    */
    simd() noexcept
    {}

    /**
     * @brief Constructs a SIMD vector by broadcasting a scalar value.
     *
     * @param value  The scalar value to broadcast into all lanes.
    */
    explicit(false) simd(value_type __value) noexcept {
        fill(__value);
    }

    ~simd() noexcept
    {}

    simd(const simd& __other) noexcept  {
        _storage = __other._storage;
    }

    simd(const storage_type& __storage) noexcept {
        _storage = __storage;
    }

    /**
     * @brief Returns a SIMD vector with all lanes set to zero.
    */
    raze_nodiscard static raze_always_inline simd zero() noexcept {
        simd __result {};

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            __chunk = _Zero<__isa, _Chunk>()();
        });

        return __result;
    }

    /**
     * @brief Returns a SIMD vector with all lanes set to `value`.
    */
    raze_nodiscard static raze_always_inline simd broadcast(value_type __value) noexcept {
        simd __result {};

        __result.__for_each_chunk([&] <class _Chunk, class _Tp> (_Chunk& __chunk, _Tp __value) raze_always_inline_lambda {
            __chunk = _Broadcast<__isa, _Chunk>()(__value);
        }, __value);

        return __result;
    }

    /**
     * @brief Fills all lanes with `value`.
     * @return Reference to `*this`.
    */
    raze_always_inline simd& fill(value_type __value) noexcept {
        return *this = broadcast(__value);
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
    raze_always_inline friend simd operator<<(const simd& __x, uint32 __shift) noexcept {
        simd __result = __x;

        __result.__for_each_chunk([&] <class _Chunk, class _Sh> (_Chunk& __chunk, _Sh __sh) raze_always_inline_lambda {
            __chunk = _Left_shift<simd::__isa, value_type>()(__chunk, __sh);
        }, __shift);

        return __result;
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
    raze_always_inline friend simd operator>>(const simd& __x, uint32 __shift) noexcept {
        simd __result = __x;

        __result.__for_each_chunk([&] <class _Chunk, class _Sh> (_Chunk& __chunk, _Sh __sh) raze_always_inline_lambda {
            __chunk = _Right_shift<simd::__isa, value_type>()(__chunk, __sh);
        }, __shift);

        return __result;
    }

    /**
     * @brief Element-wise subtraction.
    */
    template <class _RightType_>
    raze_always_inline friend simd operator-(
        const simd&         __x,
        const _RightType_&  __y) noexcept 
            requires(std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || 
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return sub(__x, __y);
    }

    /**
     * @brief Element-wise addition.
    */
    template <class _RightType_>
    raze_always_inline friend simd operator+(
        const simd&         __x,
        const _RightType_&  __y) noexcept 
            requires(std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || 
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return add(__x, __y);
    }

    /**
     * @brief Element-wise multiplication.
    */
    template <class _RightType_>
    raze_always_inline friend simd operator*(
        const simd&         __x, 
        const _RightType_&  __y) noexcept 
            requires(std::is_same_v<std::remove_cvref_t<_RightType_>, simd> ||
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return mul(__x, __y);
    }

    /**
     * @brief Element-wise division.
    */
    template <
        class _LeftType_,
        class _RightType_>
    raze_always_inline friend simd operator/(
        const _LeftType_&   __x,
        const _RightType_&  __y) noexcept requires(
            (std::is_same_v<std::remove_cvref_t<_LeftType_>, simd> && (
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type> ||
                std::is_same_v<std::remove_cvref_t<_RightType_>, simd>)) ||
            (std::is_same_v<std::remove_cvref_t<_RightType_>, simd> && (
                std::is_convertible_v<std::remove_cvref_t<_LeftType_>, value_type> ||
                std::is_same_v<std::remove_cvref_t<_LeftType_>, simd>)))
    {
        return div(__x, __y);
    }

    /**
     * @brief Bitwise AND.
    */
    raze_always_inline friend simd operator&(const simd& __x, const simd& __y) noexcept {
        simd __result = __x;

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk, const _Chunk& __value) raze_always_inline_lambda {
            __chunk = _And<simd::__isa, value_type>()(__chunk, __value);
        }, __y._storage);

        return __result;
    }

    /**
     * @brief Bitwise OR.
    */
    raze_always_inline friend simd operator|(const simd& __x,  const simd& __y) noexcept {
        simd __result = __x;

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk, const _Chunk& __value) raze_always_inline_lambda {
            __chunk = _Or<simd::__isa, value_type>()(__chunk, __value);
        }, __y._storage);

        return __result;
    }

    /**
     * @brief Bitwise XOR.
    */
    raze_always_inline friend simd operator^(const simd& __x, const simd& __y) noexcept {
        simd __result = __x;

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk, const _Chunk& __value) raze_always_inline_lambda {
            __chunk = _Xor<simd::__isa, value_type>()(__chunk, __value);
        }, __y._storage);

        return __result;
    }

    /**
     * @brief Bitwise NOT.
    */
    raze_always_inline simd operator~() const noexcept {
        simd __result = *this;

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            __chunk = _Not<__isa, value_type>()(__chunk);
        });
        
        return __result;
    }

    ///**
    // * @brief Element-wise equality comparison.
    // * @return A SIMD mask with one boolean per lane.
    //*/
    //raze_always_inline friend mask_type operator==(
    //    const simd& __left,
    //    const simd& __right) noexcept 
    //{
    //    return _Equal<__isa, __width, _Type_>()(__data(__left), __data(__right));
    //}

    ///**
    // * @brief Element-wise inequality comparison.
    // * @return A SIMD mask with one boolean per lane.
    //*/
    //raze_always_inline friend mask_type operator!=(
    //    const simd& __left, 
    //    const simd& __right) noexcept 
    //{
    //    return _Not_equal<__isa, __width, _Type_>()(__data(__left), __data(__right));
    //}

    //raze_always_inline friend mask_type operator<(
    //    const simd& __left, 
    //    const simd& __right) noexcept 
    //{
    //    return _Less<__isa, __width, _Type_>()(__data(__left), __data(__right));
    //}

    //raze_always_inline friend mask_type operator<=(
    //    const simd& __left,
    //    const simd& __right) noexcept
    //{
    //    return _Less_equal<__isa, __width, _Type_>()(__data(__left), __data(__right));
    //}

    //raze_always_inline friend mask_type operator>(
    //    const simd& __left,
    //    const simd& __right) noexcept 
    //{
    //    return _Greater<__isa, __width, _Type_>()(__data(__left), __data(__right));
    //}

    //raze_always_inline friend mask_type operator>=(
    //    const simd& __left, 
    //    const simd& __right) noexcept
    //{
    //    return _Greater_equal<__isa, __width, _Type_>()(__data(__left), __data(__right));
    //}

    raze_always_inline simd& operator>>=(uint32 __shift) noexcept {
        return *this = (*this >> __shift);
    }

    raze_always_inline simd& operator<<=(uint32 __shift) noexcept {
        return *this = (*this << __shift);
    }

    raze_always_inline simd& operator&=(const simd& __other) noexcept {
        return *this = (*this & __other);
    }

    raze_always_inline simd& operator|=(const simd& __other) noexcept {
        return *this = (*this | __other);
    }

    raze_always_inline simd& operator^=(const simd& __other) noexcept {
        return *this = (*this ^ __other);
    }

    raze_always_inline simd& operator+=(const simd& __other) noexcept {
        return *this = (*this + __other);
    }

    raze_always_inline simd& operator-=(const simd& __other) noexcept {
        return *this = (*this - __other);
    }

    raze_always_inline simd& operator*=(const simd& __other) noexcept {
        return *this = (*this * __other);
    }

    raze_always_inline simd& operator/=(const simd& __other) noexcept {
        return *this = (*this / __other);
    }

    raze_always_inline simd& operator=(const simd& __other) noexcept {
        _storage = __other._storage;
        return *this;
    }

    /**
     * @brief Returns the vector unchanged.
    */
    raze_always_inline simd operator+() const noexcept {
        return _storage;
    }

    /**
     * @brief Element-wise negation.
    */
    raze_always_inline simd operator-() const noexcept {
        simd __result = *this;

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            __chunk = _Negate<__isa, value_type>()(__chunk);
        });
        
        return __result;
    }

    /**
     * @brief Post-increment: returns the old value, increments each lane by 1.
    */
    raze_nodiscard raze_always_inline simd operator++(int) noexcept {
        simd __self = *this;
        *this += simd(1);
        return __self;
    }

    /**
     * @brief Pre-increment: increments each lane by 1.
    */
    raze_always_inline simd& operator++() noexcept {
        return *this += simd(1);
    }

    /**
     * @brief Post-decrement: returns the old value, decrements each lane by 1.
    */
    raze_always_inline simd operator--(int) noexcept {
        simd __self = *this;
        *this -= simd(1);
        return __self;
    }

    /**
     * @brief Pre-decrement: decrements each lane by 1.
    */
    raze_always_inline simd& operator--() noexcept {
        return *this -= simd(1);
    }

    /**
     * @brief returns the value of lane `i`.
     *
     * @param i  index in range [0, size()).
    */
    raze_nodiscard raze_always_inline _Type_ operator[](int32 __i) const noexcept {
        return __extract(__i);
    }

    /**
     * @brief returns a proxy reference to lane `i`, allowing modification.
    */
    raze_nodiscard raze_always_inline reference operator[](int32 __i) noexcept {
        return reference(*this, __i);
    }

    /**
     * @brief Returns the SIMD register width in bits.
    */
    raze_nodiscard static raze_always_inline constexpr int width() noexcept {
        return __width;
    }

    /**
     * @brief Returns the number of lanes in the vector.
    */
    raze_nodiscard static raze_always_inline constexpr int size() noexcept {
        return abi_type::size;
    }

    /**
     * @brief Alias for size().
    */
    raze_nodiscard static raze_always_inline constexpr int length() noexcept {
        return size();
    }

    raze_always_inline operator storage_type() const noexcept {
        return _storage;
    }

    /**
     *  @brief  Loads a SIMD vector from memory.
     *
     *  @param __first  Pointer to the source memory.
     *
     *  Performs an unconditional load of the entire vector.
    */
    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_from(_Iterator_ __first) noexcept {
        __for_each_chunk([&] <class _Chunk, class _It> (_Chunk& __chunk, _It& __current) raze_always_inline_lambda {
            __chunk = _Load<__isa, _Chunk>()(__current);
            algorithm::__advance_bytes(__current, sizeof(_Chunk));
        }, std::to_address(__first));
    }

    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_from(_Iterator_ __first, decltype(aligned)) noexcept {
        __for_each_chunk([&] <class _Chunk, class _It> (_Chunk& __chunk, _It& __current) raze_always_inline_lambda {
            __chunk = _Load<__isa, _Chunk>()(__current, aligned_policy{});
            algorithm::__advance_bytes(__current, sizeof(_Chunk));
        }, std::to_address(__first));
    }

    //template <
    //    any_iterator_or_pointer _Iterator_,
    //    simd_mask_type          _Mask_>
    //raze_always_inline void copy_from(
    //    _Iterator_      __first, 
    //    const _Mask_&   __mask, 
    //    const simd&     __source) noexcept 
    //{
    //    _storage = _Mask_load<__isa, __width, _Type_>()(
    //        std::to_address(__first), __data(__mask), __data(__source));
    //}

    //template <
    //    any_iterator_or_pointer _Iterator_,
    //    simd_mask_type          _Mask_>
    //raze_always_inline void copy_from(
    //    _Iterator_      __first, 
    //    const _Mask_&   __mask, 
    //    const simd&     __source, 
    //    decltype(aligned)) noexcept
    //{
    //    _storage = _Mask_load<__isa, __width, _Type_>()(
    //        std::to_address(__first), __data(__mask), __data(__source), aligned_policy{});
    //}

    //template <
    //    any_iterator_or_pointer _Iterator_,
    //    simd_mask_type          _Mask_>
    //raze_always_inline void copy_from(_Iterator_ __first, const _Mask_& __mask) noexcept {
    //    
    //}

    //template <
    //    any_iterator_or_pointer _Iterator_,
    //    simd_mask_type          _Mask_>
    //raze_always_inline void copy_from(_Iterator_ __first, const _Mask_& __mask, decltype(aligned)) noexcept {
    //    _storage = _Maskz_load<__isa, __width, _Type_, vector_type>()(
    //        std::to_address(__first), __data(__mask), aligned_policy{});
    //}

    /**
     *  @brief  Stores a SIMD vector to memory.
     *
     *  @param __first  Pointer to the destination memory.
     *
     *  Stores all lanes to @p __first.
    */
    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_to(_Iterator_ __first) noexcept {
        __for_each_chunk([&] <class _Chunk, class _It> (const _Chunk& __chunk, _It& __current) raze_always_inline_lambda {
            _Store<__isa>()(__current, __chunk);
            algorithm::__advance_bytes(__current, sizeof(_Chunk));
        }, std::to_address(__first));
    }

    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_to(_Iterator_ __first, decltype(aligned)) noexcept {
        __for_each_chunk([&] <class _Chunk, class _It> (const _Chunk& __chunk, _It& __current) raze_always_inline_lambda {
            _Store<__isa>()(__current, __chunk, aligned_policy{});
            algorithm::__advance_bytes(__current, sizeof(_Chunk));
        }, std::to_address(__first));
    }

   /* template <
        any_iterator_or_pointer _Iterator_,
        simd_mask_type          _Mask_>
    raze_always_inline void copy_to(_Iterator_ __first, const _Mask_& __mask) const noexcept {
        _Mask_store<__isa, __width, _Type_>()(std::to_address(__first), __data(__mask), _storage);
    }

    template <
        any_iterator_or_pointer _Iterator_,
        simd_mask_type          _Mask_>
    raze_always_inline void copy_to(_Iterator_ __first, const _Mask_& __mask, decltype(aligned)) const noexcept {
        _Mask_store<__isa, __width, _Type_>()(std::to_address(__first), __data(__mask), _storage, aligned_policy{});
    }*/

    template <class _Func_, class... _Extras_>
    raze_always_inline void __for_each_chunk(
        _Func_&&        __function, 
        _Extras_&&...   __extras) noexcept
    {
        if constexpr (__use_native) {
            __function(_storage, __extras...);
        } else {
            __execute_n_times<__simd_tuple_size<storage_type>::value>(
                _storage, std::forward<_Func_>(__function), __extras...);
        }
    }

    template <class _Func_, class... _Extras_>
    raze_always_inline void __for_each_chunk(
        _Func_&&        __function, 
        _Extras_&&...   __extras) const noexcept
    {
        if constexpr (__use_native) {
            __function(_storage, __extras...);
        } else {
            __execute_n_times<__simd_tuple_size<storage_type>::value>(
                _storage, std::forward<_Func_>(__function), __extras...);
        }
    }
private:
    raze_always_inline void __insert(
        int32       __position,
        value_type  __value) noexcept
    {
        if constexpr (__use_native) {
            _Insert<__isa>()(_storage, __position, __value);
        }
        else {
            __insert_element<__isa, _Type_>(_storage, __position, __value);
        }
    }

    raze_nodiscard raze_always_inline _Type_ __extract(int32 __i) const noexcept {
        raze_debug_assert(__i >= 0 && __i < size());

        if constexpr (__use_native) {
            return _Extract<__isa, _Type_>()(_storage, __i);
        }
        else {
            return __extract_element<__isa, _Type_>(_storage, __i);
        }
    }

    friend _Simd_element_reference;
    friend _Simd_bool_reference;
    
    storage_type _storage;
};

__RAZE_VX_NAMESPACE_END
