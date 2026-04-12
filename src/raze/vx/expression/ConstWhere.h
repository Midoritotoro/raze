#pragma once

#include <src/raze/vx/Arithmetic.h>
#include <src/raze/vx/Bitwise.h>
#include <src/raze/vx/expression/IsConstWhere.h>
#include <src/raze/vx/SimdIntegralTypesCheck.h>
#include <src/raze/vx/SimdMaskTypeCheck.h>
#include <src/raze/vx/Reduce.h>
#include <src/raze/vx/shuffle/Blend.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    class _DataparType_,
    class _MaskType_>
class _Const_where {
    static_assert(__is_valid_simd_v<_DataparType_>);
    static_assert(__is_simd_mask_v<_MaskType_>);
public:
    using datapar_type = _DataparType_;
    using mask_type = _MaskType_;
    using abi_type = typename datapar_type::abi_type;
    using value_type = typename datapar_type::value_type;
    using reference_type = typename datapar_type::reference;

    static constexpr auto __isa = datapar_type::__isa;
    static constexpr auto __width = datapar_type::__width;

    _Const_where(
        const datapar_type& __vector,
        const datapar_type& __source,
        const mask_type&    __mask) noexcept:
            _vector(__vector),
            _source(__source),
            _mask(__mask)
    {}

    _Const_where(const _Const_where&) noexcept = default;
    _Const_where& operator=(const _Const_where&) noexcept = default;
    
    ~_Const_where() noexcept = default;

    template <
        class _IteratorType_,
        class _AlignmentPolicy_>
    raze_always_inline void __store(
        _IteratorType_      __begin,
        _AlignmentPolicy_&& __policy) const noexcept
    {
        return _Mask_store<__isa, __width, value_type>()(std::to_address(__begin),
            __data(_mask), __data(_vector), __policy);
    }

    raze_nodiscard raze_always_inline auto __reduce_add() const noexcept {
        return _Reduce_add<__isa, __width, value_type>()(_Blend<__isa, __width, value_type>()(
            __data(_vector), __data(_source), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __abs() const noexcept {
        return _Mask_abs<__isa, __width, value_type>()(
            __data(_vector), __data(_mask), __data(_source));
    }

    raze_nodiscard raze_always_inline auto __horizontal_min() const noexcept {
        return _Horizontal_min<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_vector),
                __data(_source), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __horizontal_max() const noexcept {
        return _Horizontal_max<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_vector),
                __data(_source), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __vertical_min(const datapar_type& __datapar) const noexcept {
        return _Mask_vertical_min<__isa, __width, value_type>()(__data(__datapar),
            __data(_vector), __data(_mask), __data(_source));
    }

    raze_nodiscard raze_always_inline auto __vertical_max(const datapar_type& __datapar) const noexcept {
        return _Mask_vertical_max<__isa, __width, value_type>()(__data(__datapar),
            __data(_vector), __data(_mask), __data(_source));
    }

    raze_always_inline friend datapar_type operator+(
        const _Const_where& __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_add<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator+(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_add<__isa, __width, value_type>()(__data(__right._vector), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator-(
        const _Const_where& __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_sub<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator-(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_sub<__isa, __width, value_type>()(__data(__left), __data(__right._vector),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator*(
        const _Const_where& __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_mul<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator*(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_mul<__isa, __width, value_type>()(__data(__left), __data(__right._vector),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator/(
        const _Const_where& __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator/(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(__left), __data(__right._vector),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator/(
        const _Const_where& __left,
        const value_type&   __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(__left._vector), __right,
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator/(
        const value_type&   __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(datapar_type(__left)), __data(__right._vector),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator&(
        const _Const_where& __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_and<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator&(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_and<__isa, __width, value_type>()(__data(__right._vector), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator|(
        const _Const_where&     __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_or<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator|(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_or<__isa, __width, value_type>()(__data(__right._vector), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator^(
        const _Const_where& __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_xor<__isa, __width, value_type>()(__data(__left._vector), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator^(
        const datapar_type& __left,
        const _Const_where& __right) noexcept
    {
        return _Mask_xor<__isa, __width, value_type>()(__data(__right._vector), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline datapar_type operator+() const noexcept {
        return _Mask_assign<__isa, __width, value_type>()(
            __data(_vector), __data(_mask), __data(_source));
    }

    raze_always_inline datapar_type operator-() const noexcept {
        return _Mask_negate<__isa, __width, value_type>()(
            __data(_vector), __data(_mask), __data(_source));
    }

    raze_always_inline datapar_type operator~() const noexcept {
        return _Mask_not<__isa, __width, value_type>()(
            __data(_vector), __data(_mask), __data(_source));
    }

    raze_always_inline friend datapar_type operator<<(
        const _Const_where& __left,
        uint32              __shift) noexcept
    {
        return _Mask_left_shift<__isa, __width, value_type>()(
            __data(__left._vector), __shift, __data(_mask), __data(_source));
    }

    raze_always_inline friend datapar_type operator>>(
        const _Const_where& __left,
        uint32              __shift) noexcept
    {
        return _Mask_right_shift<__isa, __width, value_type>()(
            __data(__left._vector), __shift, __data(_mask), __data(_source));
    }

    raze_nodiscard raze_always_inline value_type operator[](int32 __i) const noexcept {
        return _mask[__i] ? _vector[__i] : _source[__i];
    }

    explicit raze_always_inline operator datapar_type() const noexcept {
        return _vector;
    }

    raze_nodiscard raze_always_inline datapar_type data() const noexcept {
        return _vector;
    }

    raze_nodiscard raze_always_inline datapar_type source() const noexcept {
        return _source;
    }

    raze_nodiscard raze_always_inline mask_type mask() const noexcept {
        return _mask;
    }
private:
    datapar_type _source;
    datapar_type _vector;
    mask_type _mask;
};

__RAZE_VX_NAMESPACE_END
