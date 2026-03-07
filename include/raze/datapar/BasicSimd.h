#pragma once 

#include <src/raze/datapar/Arithmetic.h>

#include <src/raze/datapar/Compare.h>
#include <src/raze/datapar/Shuffle.h>

#include <src/raze/utility/Assert.h>

#include <raze/datapar/BasicSimdMask.h>
#include <raze/datapar/SimdCast.h>

#include <src/raze/datapar/Memory.h>
#include <src/raze/datapar/Bitwise.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

using aligned_policy    = __aligned_policy;
using unaligned_policy  = __unaligned_policy;

template <
    arch::ISA	_ISA_,
    class	    _Type_,
    uint32      _Width_ = __default_width<_ISA_>>
class simd {
    static_assert(type_traits::__is_generation_supported_v<_ISA_>);
    static_assert(type_traits::__is_vector_type_supported_v<std::decay_t<_Type_>>);
public:
    static constexpr auto __isa = _ISA_;
    static constexpr auto __width = _Width_;
    
    using vector_type   = type_traits::__deduce_simd_vector_type<_ISA_, _Type_, _Width_>;
    using reference     = _Simd_element_reference<simd>;
    using value_type    = _Type_;
    using mask_type     = simd_mask<_ISA_, _Type_, _Width_>;

    template <typename _DesiredType_ = value_type>
    static constexpr inline bool is_native_mask_load_supported_v = __is_native_mask_load_supported_v<
        __isa, _Width_, sizeof(_DesiredType_)>;

    template <typename _DesiredType_ = value_type>
    static constexpr inline bool is_native_mask_store_supported_v = __is_native_mask_store_supported_v<
        __isa, _Width_, sizeof(_DesiredType_)>;

    raze_always_inline simd() noexcept
    {}

    raze_always_inline simd(const value_type __value) noexcept {
        fill(__value);
    }

    raze_always_inline ~simd() noexcept
    {}

    template <class _VectorType_>
    raze_always_inline simd(_VectorType_ __other) noexcept
        requires (__is_intrin_type_v<_VectorType_> || __is_valid_simd_v<_VectorType_>) 
    {
        _vector = simd_cast<vector_type>(__other);
    }

    raze_nodiscard static raze_always_inline simd zero() noexcept {
        return _Simd_broadcast_zeros<_ISA_, _Width_, vector_type>()();
    }

    raze_nodiscard static raze_always_inline simd broadcast(value_type __value) noexcept {
        return _Simd_broadcast<_ISA_, _Width_, vector_type>()(__value);
    }

    raze_always_inline simd& fill(value_type __value) noexcept {
        _vector = _Simd_broadcast<_ISA_, _Width_, vector_type>()(__value);
        return *this;
    }

    raze_nodiscard raze_always_inline friend simd operator+(
        const simd&      __left,
        const value_type __right) noexcept 
    {
        return _Simd_add<_ISA_, _Width_, _Type_>()(__left._vector, __data(simd(__right)));
    }

    raze_nodiscard raze_always_inline friend simd operator-(
        const simd&         __left,
        const value_type    __right) noexcept
    {
        return _Simd_sub<_ISA_, _Width_, _Type_>()(__left._vector, __data(simd(__right)));
    }

    raze_nodiscard raze_always_inline friend simd operator*(
        const simd&         __left, 
        const value_type    __right) noexcept 
    {
        return {};
    }

    raze_nodiscard raze_always_inline friend simd operator/(
        const simd&         __left, 
        const value_type    __right) noexcept 
    {
        return {};
    }

    raze_nodiscard raze_always_inline friend simd operator+(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Simd_add<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend simd operator-(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Simd_sub<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend simd operator*(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return {};
    }

    raze_nodiscard raze_always_inline friend simd operator/(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return {};
    }

    raze_nodiscard raze_always_inline friend simd operator&(
        const simd& __left,
        const simd& __right) noexcept
    {
        return _Simd_bit_and<_ISA_, _Width_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend simd operator|(
        const simd& __left, 
        const simd& __right) noexcept
    {
        return _Simd_bit_or<_ISA_, _Width_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend simd operator^(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Simd_bit_xor<_ISA_, _Width_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator==(
        const simd& __left,
        const simd& __right) noexcept 
    {
        return _Simd_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator!=(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Simd_not_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator<(
        const simd& __left, 
        const simd& __right) noexcept 
    {
        return _Simd_less<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator<=(
        const simd& __left,
        const simd& __right) noexcept
    {
        return _Simd_less_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator>(
        const simd& __left,
        const simd& __right) noexcept 
    {
        return _Simd_greater<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_nodiscard raze_always_inline friend mask_type operator>=(
        const simd& __left, 
        const simd& __right) noexcept
    {
        return _Simd_greater_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
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
        return __simd_negate<_ISA_, _Width_, _Type_>(_vector);
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
        return _Simd_bit_not<_ISA_, _Width_>()(_vector);
    }

    raze_nodiscard raze_always_inline _Type_ operator[](int32 __i) const noexcept {
        return __extract(__i);
    }

    raze_nodiscard raze_always_inline reference operator[](int32 __i) noexcept {
        return reference(*this, __i);
    }

    raze_nodiscard static raze_always_inline constexpr int width() noexcept {
        return _Width_;
    }

    raze_nodiscard static raze_always_inline constexpr int size() noexcept {
        return (sizeof(vector_type) / sizeof(_Type_));
    }

    raze_nodiscard static raze_always_inline constexpr int length() noexcept {
        return size();
    }

    raze_always_inline operator vector_type() const noexcept {
        return _vector;
    }
private:
    raze_always_inline void __insert(
        int32       __position,
        value_type  __value) noexcept
    {
        _Simd_insert<_ISA_, _Width_>()(_vector, __position, __value);
    }

    raze_nodiscard raze_always_inline _Type_ __extract(int32 __i) const noexcept {
        raze_debug_assert(__i >= 0 && __i < size());
        return _Simd_extract<_ISA_, _Width_, _Type_>()(_vector, __i);
    }

    friend _Simd_element_reference;
    friend _Simd_bool_reference;
    
    vector_type _vector;
};

__RAZE_DATAPAR_NAMESPACE_END
