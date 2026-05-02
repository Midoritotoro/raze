#pragma once

#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>
#include <src/raze/vx/expression/IsConstWhere.h>
#include <src/raze/vx/hw/Mask.h>
#include <src/raze/vx/hw/Reduce.h>
#include <src/raze/vx/hw/Merge.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    class _DataparType_,
    class _MaskType_>
class _Const_where_zero {
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

    _Const_where_zero(
        const datapar_type& __vector,
        const mask_type&    __mask) noexcept:
            _vector(__vector),
            _mask(__mask)
    {}

    _Const_where_zero(const _Const_where_zero&) noexcept = default;
    _Const_where_zero& operator=(const _Const_where_zero&) noexcept = default;

    ~_Const_where_zero() noexcept = default;

    template <
        class _IteratorType_,
        class _AlignmentPolicy_>
    raze_always_inline void __store(
        _IteratorType_      __begin,
        _AlignmentPolicy_&& __policy) const noexcept
    {
        return _Mask_store<__isa, __width, value_type>()(
            std::to_address(__begin), __data(_mask), __data(_vector), __policy);
    }

    raze_nodiscard raze_always_inline auto __reduce_add() const noexcept {
        return _Reduce_add<__isa, __width, value_type>()(_Blend<__isa, __width, value_type>()(
            __data(_vector), _Broadcast_zeros<__isa, __width, typename datapar_type::vector_type>()(), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __abs() const noexcept {
        return _Maskz_abs<__isa, __width, value_type>()(__data(_vector), __data(_mask));
    }

    raze_nodiscard raze_always_inline auto __horizontal_min() const noexcept {
        return _Horizontal_min<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_vector),
                _Broadcast_zeros<__isa, __width, typename datapar_type::vector_type>()(), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __horizontal_max() const noexcept {
        return _Horizontal_max<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_vector),
                _Broadcast_zeros<__isa, __width, typename datapar_type::vector_type>()(), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __vertical_min(const datapar_type& __vector) const noexcept {
        return _Maskz_vertical_min<__isa, __width, value_type>()(
            __data(__vector), __data(_vector), __data(_mask));
    }

    raze_nodiscard raze_always_inline auto __vertical_max(const datapar_type& __vector) const noexcept {
        return _Maskz_vertical_max<__isa, __width, value_type>()(
            __data(__vector), __data(_vector), __data(_mask));
    }

    template <u8 _TernaryMask_>
    raze_nodiscard static raze_always_inline auto __ternarylogic(
        const datapar_type&                             __x,
        const datapar_type&                             __y,
        const _Const_where_zero&                        __z,
        std::integral_constant<u8, _TernaryMask_>    __imm8) noexcept
    {
        return _Maskz_ternarylogic<__isa, __width, value_type>()(__data(__x), __data(__y), 
            __data(__z._vector), __imm8, __data(__z._mask));
    }

    template <u8 _TernaryMask_>
    raze_nodiscard static raze_always_inline auto __ternarylogic(
        const datapar_type&                             __x,
        const _Const_where_zero&                        __y,
        const datapar_type&                             __z,
        std::integral_constant<u8, _TernaryMask_>    __imm8) noexcept
    {
        return _Maskz_ternarylogic<__isa, __width, value_type>()(__data(__x), __data(__y._vector),
            __data(__z), __imm8, __data(__y._mask));
    }

    template <u8 _TernaryMask_>
    raze_nodiscard static raze_always_inline auto __ternarylogic(
        const _Const_where_zero&                        __x,
        const datapar_type&                             __y,
        const datapar_type&                             __z,
        std::integral_constant<u8, _TernaryMask_>    __imm8) noexcept
    {
        return _Maskz_ternarylogic<__isa, __width, value_type>()(__data(__x._vector), __data(__y),
            __data(__z), __imm8, __data(__x._mask));
    }

    raze_always_inline friend datapar_type operator+(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_add<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator+(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_add<__isa, __width, value_type>()(
            __data(__right._vector), __data(__left), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator-(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_sub<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator-(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_sub<__isa, __width, value_type>()(
            __data(__left), __data(__right._vector), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator*(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_mul<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator*(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_mul<__isa, __width, value_type>()(
            __data(__left), __data(__right._vector), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(__left), __data(__right._vector), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const _Const_where_zero&    __left,
        const value_type&           __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(__left._vector), __right, __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const value_type&           __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(datapar_type(__left)), __data(__right._vector), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator&(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_and<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator&(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_and<__isa, __width, value_type>()(
            __data(__right._vector), __data(__left), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator|(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_or<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator|(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_or<__isa, __width, value_type>()(
            __data(__right._vector), __data(__left), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator^(
        const _Const_where_zero&    __left,
        const datapar_type&         __right) noexcept
    {
        return _Maskz_xor<__isa, __width, value_type>()(
            __data(__left._vector), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator^(
        const datapar_type&         __left,
        const _Const_where_zero&    __right) noexcept
    {
        return _Maskz_xor<__isa, __width, value_type>()(__data(__right._vector), __data(__left),
            __data(__right._mask));
    }

    raze_always_inline datapar_type operator+() const noexcept {
        return _Maskz_assign<__isa, __width, value_type>()(
            __data(_vector), __data(_mask));
    }

    raze_always_inline datapar_type operator-() const noexcept {
        return _Maskz_negate<__isa, __width, value_type>()(
            __data(_vector), __data(_mask));
    }

    raze_always_inline datapar_type operator~() const noexcept {
        return _Maskz_not<__isa, __width, value_type>()(
            __data(_vector), __data(_mask));
    }

    raze_always_inline friend datapar_type operator<<(
        const _Const_where_zero&    __left,
        u32                      __shift) noexcept
    {
        return _Maskz_left_shift<__isa, __width, value_type>()(
            __data(__left._vector), __shift, __data(_mask));
    }

    raze_always_inline friend datapar_type operator>>(
        const _Const_where_zero&    __left,
        u32                      __shift) noexcept
    {
        return _Maskz_right_shift<__isa, __width, value_type>()(
            __data(__left._vector), __shift, __data(_mask));
    }

    raze_nodiscard raze_always_inline value_type operator[](i32 __i) const noexcept {
        return _mask[__i] ? _vector[__i] : value_type(0);
    }

    explicit raze_always_inline operator datapar_type() const noexcept {
        return _vector;
    }

    raze_nodiscard raze_always_inline datapar_type data() const noexcept {
        return _vector;
    }

    raze_nodiscard raze_always_inline mask_type mask() const noexcept {
        return _mask;
    }
private:
    datapar_type _vector;
    mask_type _mask;
};

__RAZE_VX_NAMESPACE_END
