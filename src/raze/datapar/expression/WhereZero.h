#pragma once

#include <src/raze/datapar/Arithmetic.h>
#include <src/raze/datapar/Bitwise.h>
#include <src/raze/datapar/expression/IsWhereZero.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>
#include <src/raze/datapar/SimdMaskTypeCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _DataparType_,
    class _MaskType_>
class _Where_zero {
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

    _Where_zero(
        datapar_type&          __datapar,
        const mask_type&       __mask) noexcept:
            _reference(__datapar),
            _mask(__mask)
    {}

    _Where_zero(const _Where_zero&) noexcept = default;
    _Where_zero(_Where_zero&&) noexcept = default;

    ~_Where_zero() = default;

    
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
        return _Reduce_add<__isa, __width, value_type>()(_Blend<__isa, __width, value_type>()(__data(_reference),
            _Broadcast_zeros<__isa, __width, typename datapar_type::vector_type>()(), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __abs() const noexcept {
        return _Maskz_abs<__isa, __width, value_type>()(__data(_reference), __data(_mask));
    }

    raze_nodiscard raze_always_inline auto __horizontal_min() const noexcept {
        return _Horizontal_min<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_reference),
                _Broadcast_zeros<__isa, __width, typename datapar_type::vector_type>()(), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __horizontal_max() const noexcept {
        return _Horizontal_max<__isa, __width, value_type>()(
            _Blend<__isa, __width, value_type>()(__data(_reference),
                _Broadcast_zeros<__isa, __width, typename datapar_type::vector_type>()(), __data(_mask)));
    }

    raze_nodiscard raze_always_inline auto __vertical_min(const datapar_type& __datapar) const noexcept {
        return _Maskz_vertical_min<__isa, __width, value_type>()(__data(__datapar),
            __data(_reference), __data(_mask));
    }

    raze_nodiscard raze_always_inline auto __vertical_max(const datapar_type& __datapar) const noexcept {
        return _Maskz_vertical_max<__isa, __width, value_type>()(__data(__datapar),
            __data(_reference), __data(_mask));
    }

    raze_always_inline friend datapar_type operator+(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_add<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator+(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_add<__isa, __width, value_type>()(
            __data(__right._reference), __data(__left), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator-(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_sub<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator-(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_sub<__isa, __width, value_type>()(
            __data(__left), __data(__right._reference), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator*(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_mul<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator*(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_mul<__isa, __width, value_type>()(
            __data(__left), __data(__right._reference), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(__left), __data(__right._reference), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const _Where_zero&  __left,
        const value_type&   __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(__left._reference), __right, __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator/(
        const value_type&   __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_div<__isa, __width, value_type>()(
            __data(datapar_type(__left)), __data(__right._reference), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator&(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_and<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator&(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_and<__isa, __width, value_type>()(
            __data(__right._reference), __data(__left), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator|(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_or<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator|(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_or<__isa, __width, value_type>()(
            __data(__right._reference), __data(__left), __data(__right._mask));
    }

    raze_always_inline friend datapar_type operator^(
        const _Where_zero&  __left,
        const datapar_type& __right) noexcept
    {
        return _Maskz_xor<__isa, __width, value_type>()(
            __data(__left._reference), __data(__right), __data(__left._mask));
    }

    raze_always_inline friend datapar_type operator^(
        const datapar_type& __left,
        const _Where_zero&  __right) noexcept
    {
        return _Maskz_xor<__isa, __width, value_type>()(
            __data(__right._reference), __data(__left), __data(__right._mask));
    }

    raze_always_inline datapar_type operator+() const noexcept {
        return _Maskz_assign<__isa, __width, value_type>()(
            __data(_reference), __data(_mask));
    }

    raze_always_inline datapar_type operator-() const noexcept {
        return _Maskz_negate<__isa, __width, value_type>()(
            __data(_reference), __data(_mask));
    }

    raze_always_inline datapar_type operator~() const noexcept {
        return _Maskz_not<__isa, __width, value_type>()(
            __data(_reference), __data(_mask));
    }

    raze_always_inline friend datapar_type operator<<(
        const _Where_zero&  __left,
        uint32              __shift) noexcept
    {
        return _Maskz_left_shift<__isa, __width, value_type>()(
            __data(__left._reference), __shift, __data(_mask));
    }

    raze_always_inline friend datapar_type operator>>(
        const _Where_zero&  __left,
        uint32              __shift) noexcept
    {
        return _Maskz_right_shift<__isa, __width, value_type>()(
            __data(__left._reference), __shift, __data(_mask));
    }

    raze_always_inline _Where_zero& operator=(const datapar_type& __other) noexcept {
        _reference = _Mask_assign<__isa, __width, value_type>()(
            __data(_reference), __data(_mask), __data(__other));
        return *this;
    }

    raze_always_inline _Where_zero& operator=(const _Where_zero& __other) noexcept {
        _mask = __other._mask;
        _reference = __other._reference;
        return *this;
    }

    raze_always_inline _Where_zero operator++(int) noexcept {
        _Where_zero __self = *this;
        *this += datapar_type(1);
        return __self;
    }

    raze_always_inline _Where_zero& operator++() noexcept {
        return (*this += datapar_type(1));
    }

    raze_always_inline _Where_zero operator--(int) noexcept {
        _Where_zero __self = *this;
        *this -= datapar_type(1);
        return __self;
    }

    raze_always_inline _Where_zero& operator--() noexcept {
        return (*this -= datapar_type(1));
    }

    raze_always_inline _Where_zero& operator>>=(uint32 __shift) noexcept {
        const auto __result = *this >> __shift;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator<<=(uint32 __shift) noexcept {
        const auto __result = *this << __shift;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator&=(const datapar_type& __other) noexcept {
        const auto __result = *this & __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator|=(const datapar_type& __other) noexcept {
        const auto __result = *this | __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator^=(const datapar_type& __other) noexcept {
        const auto __result = *this ^ __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator+=(const datapar_type& __other) noexcept {
        const auto __result = *this + __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator-=(const datapar_type& __other) noexcept {
        const auto __result = *this - __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator*=(const datapar_type& __other) noexcept {
        const auto __result = *this * __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline _Where_zero& operator/=(const datapar_type& __other) noexcept {
        const auto __result = *this / __other;
        _reference = __result;
        return *this;
    }

    raze_always_inline operator datapar_type&() noexcept {
        return _reference;
    }

    raze_nodiscard raze_always_inline value_type operator[](int32 __i) const noexcept {
        return _mask[__i] ? _reference[__i] : value_type(0);
    }

    raze_always_inline operator datapar_type() const noexcept {
        return static_cast<datapar_type>(_reference);
    }

    raze_nodiscard raze_always_inline datapar_type data() const noexcept {
        return _reference;
    }

    raze_nodiscard raze_always_inline datapar_type& data() noexcept {
        return _reference;
    }

    raze_nodiscard raze_always_inline mask_type mask() const noexcept {
        return _reference;
    }
private:
    datapar_type& _reference;
    mask_type _mask;
};

__RAZE_DATAPAR_NAMESPACE_END

