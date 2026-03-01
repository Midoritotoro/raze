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

    template <typename _DesiredType_>
    using __mask_type = type_traits::__deduce_simd_mask_type<_ISA_, _Type_, _Width_>;
public:
    static constexpr auto __isa = _ISA_;
    static constexpr auto __width = _Width_;
    
    using vector_type = type_traits::__deduce_simd_vector_type<_ISA_, _Type_, _Width_>;

    using value_type    = _Type_;
    using mask_type     = simd_mask<_ISA_, _Type_, _Width_>;
    using size_type     = uint8;

    template <typename _DesiredType_ = value_type>
    static constexpr inline bool is_native_mask_load_supported_v = __is_native_mask_load_supported_v<
        __isa, _Width_, sizeof(_DesiredType_)>;

    template <typename _DesiredType_ = value_type>
    static constexpr inline bool is_native_mask_store_supported_v = __is_native_mask_store_supported_v<
        __isa, _Width_, sizeof(_DesiredType_)>;

    simd() noexcept
    {}

    simd(const value_type __value) noexcept {
        fill(__value);
    }

    ~simd() noexcept
    {}

    template <
        typename _VectorType_,
        std::enable_if_t<__is_intrin_type_v<_VectorType_> || __is_valid_simd_v<_VectorType_>, int> = 0>
    simd(_VectorType_ __other) noexcept {
        _vector = simd_cast<vector_type>(__other);
    }

    static raze_always_inline simd zero() noexcept {
        return _Simd_broadcast_zeros<_ISA_, _Width_, vector_type>()();
    }

    raze_always_inline simd& fill(value_type __value) noexcept {
        _vector = _Simd_broadcast<_ISA_, _Width_, vector_type>()(__value);
        return *this;
    }

    raze_always_inline value_type extract(size_type __index) const noexcept {
        return _Simd_extract<_ISA_, _Width_, value_type>()(_vector, __index);
    }

    raze_always_inline void insert(size_type __position, value_type __value) noexcept {
        _Simd_insert<_ISA_, _Width_>()(_vector, __position, __value);
    }

    raze_always_inline friend simd operator+(const simd& __left, const value_type __right) noexcept {
        return _Simd_add<_ISA_, _Width_, _Type_>()(__left._vector, __simd_unwrap(simd(__right)));
    }

    raze_always_inline friend simd operator-(const simd& __left, const value_type __right) noexcept {
        return _Simd_sub<_ISA_, _Width_, _Type_>()(__left._vector, __simd_unwrap(simd(__right)));
    }

    raze_always_inline friend simd operator*(const simd& __left, const value_type __right) noexcept {
        return {};
    }

    raze_always_inline friend simd operator/(const simd& __left, const value_type __right) noexcept {
        return {};
    }

    raze_always_inline friend simd operator+(const simd& __left, const simd& __right) noexcept {
        return _Simd_add<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd operator-(const simd& __left, const simd& __right) noexcept {
        return _Simd_sub<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd operator*(const simd& __left, const simd& __right) noexcept {
        return {};
    }

    raze_always_inline friend simd operator/(const simd& __left, const simd& __right) noexcept {
        return {};
    }

    raze_always_inline friend simd operator&(const simd& __left, const simd& __right) noexcept {
        return _Simd_bit_and<_ISA_, _Width_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd operator|(const simd& __left, const simd& __right) noexcept {
        return _Simd_bit_or<_ISA_, _Width_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd operator^(const simd& __left, const simd& __right) noexcept {
        return _Simd_bit_xor<_ISA_, _Width_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd_mask<_ISA_, _Type_, _Width_> operator==(const simd& __left, const simd& __right) noexcept {
        return _Simd_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd_mask<_ISA_, _Type_, _Width_> operator!=(const simd& __left, const simd& __right) noexcept {
        return _Simd_not_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd_mask<_ISA_, _Type_, _Width_> operator<(const simd& __left, const simd& __right) noexcept {
        return _Simd_less<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd_mask<_ISA_, _Type_, _Width_> operator<=(const simd& __left, const simd& __right) noexcept {
        return _Simd_less_equal<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd_mask<_ISA_, _Type_, _Width_> operator>(const simd& __left, const simd& __right) noexcept {
        return _Simd_greater<_ISA_, _Width_, _Type_>()(__left._vector, __right._vector);
    }

    raze_always_inline friend simd_mask<_ISA_, _Type_, _Width_> operator>=(const simd& __left, const simd& __right) noexcept {
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

    raze_always_inline simd operator+() const noexcept {
        return _vector;
    }

    raze_always_inline simd operator-() const noexcept {
        return __simd_negate<_ISA_, _Width_, _Type_>(_vector);
    }

    raze_always_inline simd operator++(int) noexcept {
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

    raze_always_inline simd operator~() const noexcept {
        return _Simd_bit_not<_ISA_, _Width_>()(_vector);
    }

    raze_always_inline _Type_ operator[](const size_type __index) const noexcept {
        return _Simd_extract<_ISA_, _Width_, _Type_>()(_vector, __index);
    }

    static raze_always_inline constexpr int width() noexcept {
        return _Width_;
    }

    static raze_always_inline constexpr int size() noexcept {
        return (sizeof(vector_type) / sizeof(_Type_));
    }

    static raze_always_inline constexpr int length() noexcept {
        return size();
    }

    raze_always_inline vector_type unwrap() const noexcept {
        return _vector;
    }
private:
    vector_type _vector;
};

__RAZE_DATAPAR_NAMESPACE_END
