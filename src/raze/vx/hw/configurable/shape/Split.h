#pragma once 

#include <raze/options/Options.h>
#include <raze/vx/Abi.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_>
using pair_type = std::pair<simd<typename _Simd_::value_type, resize_abi_t<abi_t<_Simd_>, _Simd_::size() / 2>>,
    simd<typename _Simd_::value_type, resize_abi_t<abi_t<_Simd_>, _Simd_::size() - (_Simd_::size() / 2)>>>;

template <class _Options_>
struct _Configurable_split : raze::options::strict_elementwise_callable<_Configurable_split, _Options_> {
    template <simd_type _Type_>
    raze_nodiscard raze_always_inline pair_type<_Type_> operator()(const _Type_& __x) const noexcept
        requires(_Type_::size() > 1)
    {
        return raze::options::__dispatch_call(*this, __x);
    }

    template <simd_type _Type_>
    static raze_always_inline pair_type<_Type_> deferred_call(auto __options, const _Type_& __x) noexcept {
        using _First_ = typename pair_type<_Type_>::first_type;
        using _Second_ = typename pair_type<_Type_>::second_type;
        
        alignas(64) typename _Type_::value_type __arr[_Type_::size()];
        vx::__store[vx::aligned](__arr, __x);

        return { vx::__load<_First_>[vx::aligned](__arr), vx::__load<_Second_>[vx::aligned](__arr + _First_::size()) };
    }

    using callable_tag_type = _Configurable_split;
};

constexpr inline auto __split = raze::options::functor<_Configurable_split>;

__RAZE_VX_NAMESPACE_END
