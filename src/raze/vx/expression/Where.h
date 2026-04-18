#pragma once

#include <src/raze/vx/hw/Arithmetic.h>
#include <src/raze/vx/hw/Bitwise.h>
#include <src/raze/vx/expression/IsWhere.h>
#include <src/raze/vx/hw/Mask.h>
#include <src/raze/vx/hw/Reduce.h>
#include <src/raze/vx/hw/Merge.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
    class _DataparType_,
    class _MaskType_>
class _Where {
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

    _Where(
        datapar_type&          __datapar,
        const datapar_type&    __source,
        const mask_type&       __mask) noexcept:
            _reference(__datapar),
            _source(__source),
            _mask(__mask)
    {}

    _Where(const _Where&) noexcept = default;
    _Where(_Where&&) noexcept = default;

    ~_Where() = default;

    template <
        class _IteratorType_,
        class _AlignmentPolicy_>
    raze_always_inline void __store(
        _IteratorType_      __begin,
        _AlignmentPolicy_&& __policy) const noexcept
    {
        return _Mask_store<__isa, __width, value_type>()(std::to_address(__begin),
            __data(_mask), __data(_reference), __policy);
    }

    raze_nodiscard raze_always_inline auto __reduce_add() const noexcept {
        return _Reduce_add<__isa, __width, value_type>()(_Blend<__isa, __width, value_type>()(
            __data(_reference), __data(_source), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __abs() const noexcept {
        return _Mask_abs<__isa, __width, value_type>()(
            __data(_reference), __data(_mask), __data(_source));
    }

    raze_nodiscard raze_always_inline auto __horizontal_min() const noexcept {
        return _Horizontal_min<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_reference),
                __data(_source), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __horizontal_max() const noexcept {
        return _Horizontal_max<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_reference),
                __data(_source), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __vertical_min(const datapar_type& __datapar) const noexcept {
        return _Mask_vertical_min<__isa, __width, value_type>()(__data(__datapar),
            __data(_reference), __data(_mask), __data(_source));
    }

    raze_nodiscard raze_always_inline auto __vertical_max(const datapar_type& __datapar) const noexcept {
        return _Mask_vertical_max<__isa, __width, value_type>()(__data(__datapar),
            __data(_reference), __data(_mask), __data(_source));
    }

    template <uint8 _TernaryMask_>
    raze_nodiscard static raze_always_inline auto __ternarylogic(
        const datapar_type&                             __x,
        const datapar_type&                             __y,
        const _Where&                                   __z,
        std::integral_constant<uint8, _TernaryMask_>    __imm8) noexcept
    {
        return _Mask_ternarylogic<__isa, __width, value_type>()(__data(__x), __data(__y), 
            __data(__z._reference), __imm8, __data(__z._mask), __data(__z._source));
    }

    template <uint8 _TernaryMask_>
    raze_nodiscard static raze_always_inline auto __ternarylogic(
        const datapar_type&                             __x,
        const _Where&                                   __y,
        const datapar_type&                             __z,
        std::integral_constant<uint8, _TernaryMask_>    __imm8) noexcept
    {
        return _Mask_ternarylogic<__isa, __width, value_type>()(__data(__x), __data(__y._reference),
            __data(__z), __imm8, __data(__y._mask), __data(__y._source));
    }

    template <uint8 _TernaryMask_>
    raze_nodiscard static raze_always_inline auto __ternarylogic(
        const _Where&                                   __x,
        const datapar_type&                             __y,
        const datapar_type&                             __z,
        std::integral_constant<uint8, _TernaryMask_>    __imm8) noexcept
    {
        return _Mask_ternarylogic<__isa, __width, value_type>()(__data(__x._reference), __data(__y),
            __data(__z), __imm8, __data(__x._mask), __data(__x._source));
    }

    raze_always_inline friend datapar_type operator+(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_add<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator+(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_add<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator-(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_sub<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator-(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_sub<__isa, __width, value_type>()(__data(__left), __data(__right._reference),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator*(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_mul<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator*(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_mul<__isa, __width, value_type>()(__data(__left), __data(__right._reference),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator/(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator/(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(__left), __data(__right._reference),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator/(
        const _Where&       __left,
        const value_type&   __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(__left._reference), __right,
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator/(
        const value_type&   __left,
        const _Where&       __right) noexcept
    {
        return _Mask_div<__isa, __width, value_type>()(__data(datapar_type(__left)), __data(__right._reference),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator&(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_and<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator&(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_and<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator|(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_or<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator|(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_or<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline friend datapar_type operator^(
        const _Where&       __left,
        const datapar_type& __right) noexcept
    {
        return _Mask_xor<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
            __data(__left._mask), __data(__left._source));
    }

    raze_always_inline friend datapar_type operator^(
        const datapar_type& __left,
        const _Where&       __right) noexcept
    {
        return _Mask_xor<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
            __data(__right._mask), __data(__right._source));
    }

    raze_always_inline datapar_type operator+() const noexcept {
        return _Mask_assign<__isa, __width, value_type>()(
            __data(_reference), __data(_mask), __data(_source));
    }

    raze_always_inline datapar_type operator-() const noexcept {
        return _Mask_negate<__isa, __width, value_type>()(
            __data(_reference), __data(_mask), __data(_source));
    }

    raze_always_inline datapar_type operator~() const noexcept {
        return _Mask_not<__isa, __width, value_type>()(
            __data(_reference), __data(_mask), __data(_source));
    }

    raze_always_inline friend datapar_type operator<<(
        const _Where&   __left,
        uint32          __shift) noexcept
    {
        return _Mask_left_shift<__isa, __width, value_type>()(
            __data(__left._reference), __shift, __data(_mask), __data(_source));
    }

    raze_always_inline friend datapar_type operator>>(
        const _Where&   __left,
        uint32          __shift) noexcept
    {
        return _Mask_right_shift<__isa, __width, value_type>()(
            __data(__left._reference), __shift, __data(_mask), __data(_source));
    }

    raze_always_inline _Where& operator=(const datapar_type& __other) noexcept {
        _reference = _Mask_assign<__isa, __width, value_type>()(__data(_reference), __data(_mask), __data(__other));
        return *this;
    }

    raze_always_inline _Where& operator=(const _Where& __other) noexcept {
        _mask = __other._mask;
        _reference = __other._reference;
        _source = __other._source;
        return *this;
    }

    raze_always_inline _Where& operator>>=(uint32 __shift) noexcept {
        const auto __result = *this >> __shift;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator<<=(uint32 __shift) noexcept {
        const auto __result = *this << __shift;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator&=(const datapar_type& __other) noexcept {
        const auto __result = *this & __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator|=(const datapar_type& __other) noexcept {
        const auto __result = *this | __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator^=(const datapar_type& __other) noexcept {
        const auto __result = *this ^ __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator+=(const datapar_type& __other) noexcept {
        const auto __result = *this + __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator-=(const datapar_type& __other) noexcept {
        const auto __result = *this - __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator*=(const datapar_type& __other) noexcept {
        const auto __result = *this * __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where& operator/=(const datapar_type& __other) noexcept {
        const auto __result = *this / __other;
        _reference = __result;
        return *this;
    }

    raze_nodiscard raze_always_inline value_type operator[](int32 __i) const noexcept {
        return _mask[__i] ? _reference[__i] : _source[__i];
    }

    raze_nodiscard raze_always_inline reference_type operator[](int32 __i) noexcept {
        return _mask[__i] ? _reference[__i] : _source[__i];
    }

    explicit raze_always_inline operator datapar_type&() noexcept {
        return _reference;
    }

    explicit raze_always_inline operator datapar_type() const noexcept {
        return static_cast<datapar_type>(_reference);
    }

    raze_nodiscard raze_always_inline datapar_type data() const noexcept {
        return _reference;
    }

    raze_nodiscard raze_always_inline datapar_type& data() noexcept {
        return _reference;
    }

    raze_nodiscard raze_always_inline datapar_type source() const noexcept {
        return _source;
    }

    raze_nodiscard raze_always_inline mask_type mask() const noexcept {
        return _mask;
    }
private:
    datapar_type _source;
    datapar_type& _reference;
    mask_type _mask;
};

__RAZE_VX_NAMESPACE_END
