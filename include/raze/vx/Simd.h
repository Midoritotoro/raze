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
 * @tparam _Type_  The scalar element type (e.g., i32, f32, f64).
 * @tparam _Abi_   ABI descriptor specifying ISA and register width.
*/
template <class _Type_, class _Abi_>
class raze_aligned_type(64) simd {
    static_assert(type_traits::__is_vector_type_supported_v<std::decay_t<_Type_>>, "Unsupported element type. ");
public:
    static constexpr auto __isa = _Abi_::isa;
    static constexpr auto __width = (_Abi_::size * sizeof(_Type_) * 8);
    static constexpr auto __size = _Abi_::size;

    using storage_type  = _Vector_storage<_Type_, _Abi_>;
    using reference     = _Simd_element_reference<simd>;
    using value_type    = _Type_;
    using mask_type     = simd_mask<_Type_, _Abi_>;
    using abi_type      = _Abi_;

    /**
     * @brief Constructs an uninitialized SIMD vector.
     *
     * The contents of the vector are unspecified. Use `fill()` or `broadcast()`
     * to initialize all lanes explicitly.
    */
    raze_always_inline simd() noexcept
    {}

    /**
     * @brief Constructs a SIMD vector by broadcasting a scalar value.
     *
     * @param value  The scalar value to broadcast into all lanes.
    */
    raze_always_inline explicit(false) simd(value_type __value) noexcept {
        fill(__value);
    }

    ~simd() noexcept
    {}

    raze_always_inline simd(const simd& __other) noexcept  {
        _storage = __other._storage;
    }

    raze_always_inline simd(const storage_type& __storage) noexcept {
        _storage = __storage;
    }

    /**
     * @brief Returns a SIMD vector with all lanes set to zero.
    */
    raze_nodiscard static raze_always_inline simd zero() noexcept {
        simd __result {};

        __result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            using _StorageType = std::remove_cvref_t<decltype(__storage_unwrap(__chunk))>;
            __chunk = _Zero<__isa, _StorageType>()();
        });

        return __result;
    }

    /**
     * @brief Returns a SIMD vector with all lanes set to `value`.
    */
    raze_nodiscard static raze_always_inline simd broadcast(value_type __value) noexcept {
        simd __result {};
        
        __result.__for_each_chunk([&] <class _Chunk, class _Tp> (_Chunk& __chunk, _Tp __value) raze_always_inline_lambda {
            using _StorageType = std::remove_cvref_t<decltype(__storage_unwrap(__chunk))>;
            __chunk = _Broadcast<__isa, _StorageType>()(__value);
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
    raze_always_inline friend simd operator<<(const simd& __x, u32 __shift) noexcept {
        return bit_shl(__x, __shift);
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
    raze_always_inline friend simd operator>>(const simd& __x, u32 __shift) noexcept {
        return bit_shr(__x, __shift);
    }

    template <class _RightType_>
    raze_always_inline friend simd operator-(
        const simd&         __x,
        const _RightType_&  __y) noexcept 
            requires(std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || 
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return sub(__x, __y);
    }

    template <class _RightType_>
    raze_always_inline friend simd operator+(
        const simd&         __x,
        const _RightType_&  __y) noexcept 
            requires(std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || 
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return add(__x, __y);
    }

    template <class _RightType_>
    raze_always_inline friend simd operator*(
        const simd&         __x, 
        const _RightType_&  __y) noexcept 
            requires(std::is_same_v<std::remove_cvref_t<_RightType_>, simd> ||
                std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return mul(__x, __y);
    }

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

    raze_always_inline friend simd operator&(const simd& __x, const simd& __y) noexcept {
        return bit_and(__x, __y);
    }

    raze_always_inline friend simd operator|(const simd& __x,  const simd& __y) noexcept {
        return bit_or(__x, __y);
    }

    raze_always_inline friend simd operator^(const simd& __x, const simd& __y) noexcept {
        return bit_xor(__x, __y);
    }

    raze_always_inline simd operator~() const noexcept {
        return bit_not(*this);
    }

    raze_always_inline friend mask_type operator==(const simd& __x, const simd& __y) noexcept {
        return is_equal(__x, __y);
    }

    raze_always_inline friend mask_type operator!=(const simd& __x, const simd& __y) noexcept {
        return is_not_equal(__x, __y);
    }

    raze_always_inline friend mask_type operator<(const simd& __x, const simd& __y) noexcept {
        return is_less(__x, __y);
    }

    raze_always_inline friend mask_type operator<=(const simd& __x, const simd& __y) noexcept {
        return is_less_equal(__x, __y);
    }

    raze_always_inline friend mask_type operator>(const simd& __x, const simd& __y) noexcept {
        return is_greater(__x, __y);
    }

    raze_always_inline friend mask_type operator>=(const simd& __x, const simd& __y) noexcept {
        return is_greater_equal(__x, __y);
    }

    raze_always_inline simd& operator>>=(u32 __shift) noexcept {
        return *this = (*this >> __shift);
    }

    raze_always_inline simd& operator<<=(u32 __shift) noexcept {
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

    raze_always_inline simd operator+() const noexcept {
        return _storage;
    }

    raze_always_inline simd operator-() const noexcept {
        return neg(*this);
    }

    raze_nodiscard raze_always_inline simd operator++(int) noexcept {
        simd __self = *this;
        *this += simd(1);
        return __self;
    }

    raze_always_inline simd& operator++() noexcept {
        return *this += simd(1);
    }

    raze_always_inline simd operator--(int) noexcept {
        simd __self = *this;
        *this -= simd(1);
        return __self;
    }

    raze_always_inline simd& operator--() noexcept {
        return *this -= simd(1);
    }

    raze_nodiscard raze_always_inline _Type_ operator[](i32 __i) const noexcept {
        return __extract(__i);
    }

    raze_nodiscard raze_always_inline reference operator[](i32 __i) noexcept {
        return reference(*this, __i);
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

    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_from(_Iterator_ __first) noexcept {
        __for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            __chunk = _Load<__isa, decltype(__chunk_data)>()(__mem);
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        });
    }

    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_from(_Iterator_ __first, decltype(aligned)) noexcept {
        __for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            __chunk = _Load<__isa, decltype(__chunk_data)>()(__mem, aligned_policy{});
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        });
    }

    template <any_iterator_or_pointer _Iterator_, simd_mask_type _Mask_>
    raze_always_inline void copy_from(_Iterator_ __first, const _Mask_& __mask, const simd& __source) noexcept {
        __for_each_chunk([&] <class _Chunk, class _Source, class _Mask> (_Chunk& __chunk, const _Source& __src_chunk, const _Mask& __mask_chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            __chunk = _Mask_load<__isa, value_type>()(__mem, __storage_unwrap(__mask_chunk), __storage_unwrap(__src_chunk));
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        }, __mask.__storage().storage(), __source.__storage().storage());
    }

    template <any_iterator_or_pointer _Iterator_, simd_mask_type _Mask_>
    raze_always_inline void copy_from(_Iterator_ __first, const _Mask_& __mask, const simd& __source, decltype(aligned)) noexcept {
        __for_each_chunk([&] <class _Chunk, class _Source, class _Mask> (_Chunk& __chunk, const _Source& __src_chunk, const _Mask& __mask_chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            __chunk = _Mask_load<__isa, value_type>()(__mem, __storage_unwrap(__mask_chunk), __storage_unwrap(__src_chunk), aligned_policy{});
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        }, __mask.__storage().storage(), __source.__storage().storage());
    }

    template <any_iterator_or_pointer _Iterator_, simd_mask_type _Mask_>
    raze_always_inline void copy_from(_Iterator_ __first, const _Mask_& __mask) noexcept {
        __for_each_chunk([&] <class _Chunk, class _Mask> (_Chunk& __chunk, const _Mask& __mask_chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            __chunk = _Maskz_load<__isa, typename _Chunk::unwrapped_type, value_type>()(__mem, __storage_unwrap(__mask_chunk), aligned_policy{});
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        }, __mask.__storage().storage());
    }

    template <any_iterator_or_pointer _Iterator_, simd_mask_type _Mask_>
    raze_always_inline void copy_from(_Iterator_ __first, const _Mask_& __mask, decltype(aligned)) noexcept {
        __for_each_chunk([&] <class _Chunk, class _Mask> (_Chunk& __chunk, const _Mask& __mask_chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            __chunk = _Maskz_load<__isa, typename _Chunk::unwrapped_type, value_type>()(__mem, __storage_unwrap(__mask_chunk), aligned_policy{});
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        }, __mask.__storage().storage());
    }

    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_to(_Iterator_ __first) noexcept {
        __for_each_chunk([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            _Store<__isa>()(__mem, __chunk_data);
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        });
    }

    template <any_iterator_or_pointer _Iterator_>
    raze_always_inline void copy_to(_Iterator_ __first, decltype(aligned)) noexcept {
        __for_each_chunk([&] <class _Chunk> (const _Chunk& __chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            _Store<__isa>()(__mem, __chunk_data, aligned_policy{});
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        });
    }

    template <any_iterator_or_pointer _Iterator_, simd_mask_type _Mask_>
    raze_always_inline void copy_to(_Iterator_ __first, const _Mask_& __mask) const noexcept {
        __for_each_chunk([&] <class _Chunk, class _MaskChunk> (const _Chunk& __chunk, const _MaskChunk& __mask_chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            _Mask_store<__isa, value_type>()(__mem, __storage_unwrap(__mask_chunk), __chunk_data);
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        }, __mask.__storage().storage());
    }

    template <any_iterator_or_pointer _Iterator_, simd_mask_type _Mask_>
    raze_always_inline void copy_to(_Iterator_ __first, const _Mask_& __mask, decltype(aligned)) const noexcept {
        __for_each_chunk([&] <class _Chunk, class _MaskChunk> (const _Chunk& __chunk, const _MaskChunk& __mask_chunk) raze_always_inline_lambda {
            auto __chunk_data = __storage_unwrap(__chunk);
            auto __mem = std::to_address(__first);
            _Mask_store<__isa, value_type>()(__mem, __storage_unwrap(__mask_chunk), __chunk_data, aligned_policy{});
            algorithm::__seek_possibly_wrapped_iterator(__first, algorithm::__bytes_pointer_offset(__mem, sizeof(value_type) * _Chunk::size));
        }, __mask.__storage().storage());
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) noexcept {
        _storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }

    template <class _Function_, class ... _Args_>
    raze_always_inline void __for_each_chunk(_Function_&& __f, _Args_&& ... __args) const noexcept {
        _storage.__for_each_chunk(std::forward<_Function_>(__f), std::forward<_Args_>(__args)...);
    }
private:
    raze_always_inline void __insert(i32 __position, value_type __value) noexcept {
        _storage.__insert(__position, __value);
    }

    raze_nodiscard raze_always_inline _Type_ __extract(i32 __i) const noexcept {
        return _storage.__extract(__i);
    }

    friend _Simd_element_reference;
    storage_type _storage;
};

__RAZE_VX_NAMESPACE_END
