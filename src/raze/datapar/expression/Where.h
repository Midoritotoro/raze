#pragma once

#include <src/raze/datapar/Arithmetic.h>
#include <src/raze/datapar/Bitwise.h>
#include <src/raze/datapar/expression/IsWhere.h>
#include <src/raze/datapar/SimdIntegralTypesCheck.h>
#include <src/raze/datapar/SimdMaskTypeCheck.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    class _DataparType_,
    class _MaskType_>
class _Where {
    static_assert(__is_valid_simd_v<_DataparType_>);
    static_assert(__is_simd_mask_v<_MaskType_>);
public:
    using datapar_type  = _DataparType_;
    using mask_type     = _MaskType_;
    using value_type    = typename datapar_type::value_type;

    static constexpr auto __isa = datapar_type::__isa;
    static constexpr auto __width = datapar_type::__width;

    _Where(
        datapar_type&          __datapar,
        const datapar_type&    __additional_source,
        const mask_type&       __mask) noexcept:
            _reference(__datapar),
            _source(__additional_source),
            _mask(__mask)
    {}

    _Where(const _Where&) noexcept = default;
    _Where(_Where&&) noexcept = default;

    ~_Where() = default;

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

    //raze_always_inline friend datapar_type operator&(
    //    const _Where&       __left,
    //    const datapar_type& __right) noexcept
    //{
    //    return _Mask_and<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
    //        __data(__left._mask), __data(__left._source));
    //}

    //raze_always_inline friend datapar_type operator&(
    //    const datapar_type& __left,
    //    const _Where&       __right) noexcept
    //{
    //    return _Mask_and<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
    //        __data(__right._mask), __data(__right._source));
    //}

    //raze_always_inline friend datapar_type operator|(
    //    const _Where&       __left,
    //    const datapar_type& __right) noexcept
    //{
    //    return _Mask_or<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
    //        __data(__left._mask), __data(__left._source));
    //}

    //raze_always_inline friend datapar_type operator|(
    //    const datapar_type& __left,
    //    const _Where&       __right) noexcept
    //{
    //    return _Mask_or<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
    //        __data(__right._mask), __data(__right._source));
    //}

    //raze_always_inline friend datapar_type operator^(
    //    const _Where&       __left,
    //    const datapar_type& __right) noexcept
    //{
    //    return _Mask_xor<__isa, __width, value_type>()(__data(__left._reference), __data(__right),
    //        __data(__left._mask), __data(__left._source));
    //}

    //raze_always_inline friend datapar_type operator^(
    //    const datapar_type& __left,
    //    const _Where&       __right) noexcept
    //{
    //    return _Mask_xor<__isa, __width, value_type>()(__data(__right._reference), __data(__left),
    //        __data(__right._mask), __data(__right._source));
    //}
private:
    datapar_type _source;
    datapar_type& _reference;
    mask_type _mask;
};

__RAZE_DATAPAR_NAMESPACE_END
