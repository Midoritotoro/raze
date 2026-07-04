#pragma once 

#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/utility/Assert.h>

#pragma strict_gs_check(off)

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_, class _Int_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left_merge_fallback(
    const _Simd_& __x, const _Simd_& __y, _Int_ __sh) noexcept 
{
    alignas(sizeof(_Simd_)) typename _Simd_::value_type __arr[_Simd_::size() * 2];

    vx::__store[vx::aligned](__arr, __x);
    vx::__store[vx::aligned](__arr + _Simd_::size(), __y);

    return vx::__load<_Simd_>[vx::aligned](__arr + __sh);
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, sizetype _Slide_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Intrin_ __slide_left_merge_native(
    _Intrin_ __x, _Intrin_ __y, std::integral_constant<sizetype, _Slide_> __sh) noexcept
{
    constexpr auto __shift_bytes = __sh * sizeof(_Type_);
    constexpr auto __size = sizeof(_Intrin_) / sizeof(_Type_);

    if constexpr (sizeof(_Intrin_) == 16) {
        if constexpr (__has_ssse3_support_v<_ISA_>) return __as<_Intrin_>(_mm_alignr_epi8(__as<__m128i>(__y), __as<__m128i>(__x), __shift_bytes));
        else {
            __x = __as<_Intrin_>(_mm_srli_si128(__as<__m128i>(__x), __shift_bytes));
            __y = __as<_Intrin_>(_mm_slli_si128(__as<__m128i>(__y), 16 - __shift_bytes));
            return __as<_Intrin_>(_mm_or_si128(__as<__m128i>(__x), __as<__m128i>(__y)));
        }
    }
    else {
        alignas(sizeof(_Intrin_)) _Type_ __arr[__size * 2];

        _Store<_ISA_>()(__arr, __x, __aligned_policy{});
        _Store<_ISA_>()(__arr + __size, __y, __aligned_policy{});

        return _Load<_ISA_, _Intrin_>()(__arr + __sh, __aligned_policy{});
    }
}

template <simd_type _Simd_, sizetype _Slide_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left_merge(
    const _Simd_& __x, const _Simd_& __y, std::integral_constant<sizetype, _Slide_> __slide) noexcept
{
    using _Abi_ = abi_t<_Simd_>;
    using _Type_ = typename _Simd_::value_type;

    if constexpr (_Slide_ == 0) return __x;
    else if constexpr (native<_Simd_>) {
        auto __r = __x;

        auto& __storage = __r.template __get<0>();
        __storage = __slide_left_merge_native<_Abi_::isa, _Type_>(__storage_unwrap(__x.template __get<0>()),
            __storage_unwrap(__y.template __get<0>()), __slide);

        return __r;
    }
    else return __slide_left_merge_fallback(__x, __y, __slide);
}

template <simd_type _Simd_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_left_merge(
    const _Simd_& __x, const _Simd_& __y, i32 __sh) noexcept 
{
    raze_debug_assert(__sh < _Simd_::size());
	return __slide_left_merge_fallback(__x, __y, __sh);
}

__RAZE_VX_NAMESPACE_END

#pragma strict_gs_check(on)