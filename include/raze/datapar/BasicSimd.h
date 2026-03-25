#pragma once 

#include <src/raze/datapar/Arithmetic.h>

#include <src/raze/datapar/Compare.h>
#include <src/raze/datapar/Shuffle.h>

#include <src/raze/utility/Assert.h>

#include <raze/datapar/BasicSimdMask.h>
#include <raze/datapar/SimdCast.h>

#include <src/raze/datapar/Memory.h>
#include <src/raze/datapar/Bitwise.h>

#include <raze/datapar/Abi.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

using aligned_policy    = __aligned_policy;
using unaligned_policy  = __unaligned_policy;

template <
    class _Type_,
    class _Abi_>
class simd {
    static_assert(type_traits::__is_vector_type_supported_v<std::decay_t<_Type_>>, "Unsupported element type. ");
public:
    static constexpr auto __isa = _Abi_::isa;
    static constexpr auto __width = _Abi_::width;
    
    using vector_type   = type_traits::__deduce_simd_vector_type<_Abi_::isa, _Type_, _Abi_::width>;
    using reference     = _Simd_element_reference<simd>;
    using value_type    = _Type_;
    using mask_type     = simd_mask<_Type_, _Abi_>;
    using abi_type      = _Abi_;

    static constexpr inline bool is_native_mask_load_supported_v = __is_native_mask_load_supported_v<__isa, __width, sizeof(value_type)>;
    static constexpr inline bool is_native_mask_store_supported_v = __is_native_mask_store_supported_v<__isa, __width, sizeof(value_type)>;

    simd() noexcept
    {}

    simd(const value_type __value) noexcept {
        fill(__value);
    }

    ~simd() noexcept
    {}

    template <class _VectorType_>
    simd(_VectorType_ __other) noexcept
        requires (__is_intrin_type_v<_VectorType_> || __is_valid_simd_v<_VectorType_>) 
    {
        _vector = simd_cast<vector_type>(__other);
    }

    raze_nodiscard static raze_always_inline simd zero() noexcept {
        return _Broadcast_zeros<__isa, __width, vector_type>()();
    }

    raze_nodiscard static raze_always_inline simd broadcast(value_type __value) noexcept {
        return _Broadcast<__isa, __width, vector_type>()(__value);
    }

    raze_always_inline simd& fill(value_type __value) noexcept {
        _vector = _Broadcast<__isa, __width, vector_type>()(__value);
        return *this;
    }

    raze_nodiscard raze_always_inline friend simd operator<<(
        const simd& __left,
        uint32      __shift) noexcept
    {
        return _Left_shift<__isa, __width, _Type_>()(__left._vector, __shift);
    }

    raze_nodiscard raze_always_inline friend simd operator>>(
        const simd& __left,
        uint32      __shift) noexcept
    {
        return _Right_shift<__isa, __width, _Type_>()(__left._vector, __shift);
    }

    template <class _RightType_>
    raze_nodiscard raze_always_inline friend simd operator-(
        const simd& __left,
        const _RightType_& __right) noexcept requires(
            std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return _Sub<__isa, __width, _Type_>()(__left._vector, __data(simd(__right)));
    }

    template <class _RightType_>
    raze_nodiscard raze_always_inline friend simd operator+(
        const simd&         __left,
        const _RightType_&  __right) noexcept requires(
            std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return _Add<__isa, __width, _Type_>()(__left._vector, __data(simd(__right)));
    }

    template <class _RightType_>
    raze_nodiscard raze_always_inline friend simd operator*(
        const simd&         __left, 
        const _RightType_& __right) noexcept requires(
            std::is_same_v<std::remove_cvref_t<_RightType_>, simd> || std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return _Mul<__isa, __width, _Type_>()(__left._vector, __data(simd(__right)));
    }

    template <class _RightType_>
    raze_nodiscard raze_always_inline friend simd operator/(
        const simd&         __left,
        const _RightType_&  __right) noexcept requires(
            std::is_same_v<std::remove_cvref_t<_RightType_>, simd>)
    {
        auto __temp = simd();

        for (auto __i = 0; __i < size(); ++__i)
            __temp[__i] = __left[__i] / __right[__i];

        return __temp;
    }

    template <class _RightType_>
    raze_nodiscard raze_always_inline friend simd operator/(
        const simd&         __left,
        const _RightType_&  __right) noexcept requires(
            std::is_convertible_v<std::remove_cvref_t<_RightType_>, value_type>)
    {
        return _Div<__isa, __width, _Type_>()(__left._vector, __data(simd(__right)));
    }

    raze_nodiscard raze_always_inline friend simd operator&(
        const simd& __left,
        const simd& __right) noexcept
    {
        return _And<__isa, __width>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend simd operator|(
        const simd& __left, 
        const simd& __right) noexcept
    {
        return _Or<__isa, __width>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend simd operator^(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Xor<__isa, __width>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator==(
        const simd& __left,
        const simd& __right) noexcept 
    {
        return _Equal<__isa, __width, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator!=(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Not_equal<__isa, __width, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator<(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Less<__isa, __width, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator<=(
        const simd& __left,
        const simd& __right) noexcept
    {
        return _Less_equal<__isa, __width, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator>(
        const simd& __left,
        const simd& __right) noexcept 
    {
        return _Greater<__isa, __width, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator>=(
        const simd& __left, 
        const simd& __right) noexcept
    {
        return _Greater_equal<__isa, __width, _Type_>()(__left._vector, __right._vector);
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
        _vector = __other._vector;
        return *this;
    }

    raze_nodiscard raze_always_inline simd operator+() const noexcept {
        return _vector;
    }

    raze_nodiscard raze_always_inline simd operator-() const noexcept {
        return _Negate<__isa, __width, _Type_>()(_vector);
    }

    raze_nodiscard raze_always_inline simd operator++(int) noexcept {
        simd __self = *this;
        *this += simd(1);
        return __self;
    }

    raze_always_inline simd& operator++() noexcept {
        return *this += simd(1);
    }

    raze_nodiscard raze_always_inline simd operator--(int) noexcept {
        simd __self = *this;
        *this -= simd(1);
        return __self;
    }

    raze_always_inline simd& operator--() noexcept {
        return *this -= simd(1);
    }

    raze_nodiscard raze_always_inline simd operator~() const noexcept {
        return _Not<__isa, __width>()(_vector);
    }

    raze_nodiscard raze_always_inline _Type_ operator[](int32 __i) const noexcept {
        return __extract(__i);
    }

    raze_nodiscard raze_always_inline reference operator[](int32 __i) noexcept {
        return reference(*this, __i);
    }

    raze_nodiscard static raze_always_inline constexpr int width() noexcept {
        return __width;
    }

    raze_nodiscard static raze_always_inline constexpr int size() noexcept {
        return (sizeof(vector_type) / sizeof(_Type_));
    }

    raze_nodiscard static raze_always_inline constexpr int length() noexcept {
        return size();
    }

    raze_nodiscard static raze_always_inline constexpr int registers_count() noexcept {
        return _Abi_::registers_count;
    }

    raze_always_inline operator vector_type() const noexcept {
        return _vector;
    }
private:
    raze_always_inline void __insert(
        int32       __position,
        value_type  __value) noexcept
    {
        _Insert<__isa, __width>()(_vector, __position, __value);
    }

    raze_nodiscard raze_always_inline _Type_ __extract(int32 __i) const noexcept {
        raze_debug_assert(__i >= 0 && __i < size());
        return _Extract<__isa, __width, _Type_>()(_vector, __i);
    }

    friend _Simd_element_reference;
    friend _Simd_bool_reference;
    
    vector_type _vector;
};

__RAZE_DATAPAR_NAMESPACE_END
