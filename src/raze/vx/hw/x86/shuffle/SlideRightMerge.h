#pragma once 

#include <src/raze/vx/hw/x86/shuffle/SlideLeft.h>
#include <src/raze/utility/Assert.h>

#pragma strict_gs_check(off)

__RAZE_VX_NAMESPACE_BEGIN

template <simd_type _Simd_, class _Int_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_right_merge_fallback(
    const _Simd_& __x, const _Simd_& __y, _Int_ __sh) noexcept 
{
    alignas(sizeof(_Simd_)) typename _Simd_::value_type __arr[_Simd_::size() * 2];

    vx::__store[vx::aligned](__arr, __y);
    vx::__store[vx::aligned](__arr + _Simd_::size(), __x);

    return vx::__load<_Simd_>[vx::aligned](__arr + (_Simd_::size() - __sh));
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, sizetype _Slide_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Intrin_ __slide_right_merge_native(
    _Intrin_ __x, _Intrin_ __y, std::integral_constant<sizetype, _Slide_> __sh) noexcept
{
    constexpr auto __shift_bytes = __sh * sizeof(_Type_);
    constexpr auto __size = sizeof(_Intrin_) / sizeof(_Type_);

    if constexpr (sizeof(_Intrin_) == 16) {
        if constexpr (__has_ssse3_support_v<_ISA_>) return __as<_Intrin_>(_mm_alignr_epi8(__as<__m128i>(__x), __as<__m128i>(__y), 16 - __shift_bytes));
        else {
            __x = __as<_Intrin_>(_mm_slli_si128(__as<__m128i>(__x), __shift_bytes));
            __y = __as<_Intrin_>(_mm_srli_si128(__as<__m128i>(__y), 16 - __shift_bytes));
            return __as<_Intrin_>(_mm_or_si128(__as<__m128i>(__x), __as<__m128i>(__y)));
        }
    }
    else if constexpr (sizeof(_Intrin_) == 32) {
        if constexpr (__has_avx512vl_support_v<_ISA_> && (__shift_bytes % 4) == 0) {
            return __as<_Intrin_>(_mm256_alignr_epi32(__as<__m256i>(__x), __as<__m256i>(__y), 8 - __shift_bytes / 4));
        }
        else if constexpr (__has_avx2_support_v<_ISA_>) {
            const auto __mid = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__y), 0x03);

            if constexpr (__shift_bytes == 16) return __as<_Intrin_>(__mid);
            else if constexpr (__shift_bytes < 16) return __as<_Intrin_>(_mm256_alignr_epi8(__as<__m256i>(__x), __mid, 16 - __shift_bytes));
            else return __as<_Intrin_>(_mm256_alignr_epi8(__mid, __as<__m256i>(__y), 32 - __shift_bytes));
        }
        else if constexpr ((__shift_bytes % 8) == 0) {
            const auto __mid = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__y), 0x03);

            if constexpr (__shift_bytes == 8) return __as<_Intrin_>(_mm256_shuffle_pd(__as<__m256d>(__mid), __as<__m256d>(__x), 0b0101));
            else if constexpr (__shift_bytes == 16) return __as<_Intrin_>(__mid);
            else if constexpr (__shift_bytes == 24) return __as<_Intrin_>(_mm256_shuffle_pd(__as<__m256d>(__y), __as<__m256d>(__mid), 0b0101));
        }
        else if constexpr (__shift_bytes < 16) {
#if defined(raze_cpp_msvc_only)
            const auto __low_x = __as<__m128i>(_mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0));
#else
            const auto __low_x = __as<__m128i>(__x);
#endif // defined(raze_cpp_msvc_only)
            const auto __low = __as<__m256i>(_mm_alignr_epi8(__low_x, _mm256_extractf128_si256(__as<__m256i>(__y), 1), 16 - __shift_bytes));
            const auto __high = _mm_alignr_epi8(_mm256_extractf128_si256(__as<__m256i>(__x), 1), __low_x, 16 - __shift_bytes);
            return __as<_Intrin_>(_mm256_insertf128_si256(__low, __high, 1));
        }
        else {
            constexpr auto __shift = ((__size - __sh) * sizeof(_Type_));
#if defined(raze_cpp_msvc_only)
            const auto __low_x = __as<__m128i>(_mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0));
            const auto __low_y = __as<__m128i>(_mm256_permute2x128_si256(__as<__m256i>(__y), __as<__m256i>(__y), 0));
#else
            const auto __low_x = __as<__m128i>(__x);
            const auto __low_y = __as<__m128i>(__y);
#endif // defined(raze_cpp_msvc_only)
            const auto __high_y = _mm256_extractf128_si256(__as<__m256i>(__y), 1);
            const auto __low = __as<__m256i>(_mm_alignr_epi8(__high_y, __low_y, __shift));
            const auto __high = _mm_alignr_epi8(__low_x, __high_y, __shift);
            return __as<_Intrin_>(_mm256_insertf128_si256(__low, __high, 1));
        }
    }
    else if constexpr (sizeof(_Intrin_) == 64) {
        if constexpr ((__shift_bytes % 4) == 0) {
            return __as<_Intrin_>(_mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 16 - __shift_bytes / 4));
        }
        else if constexpr (__shift_bytes < 16) {
            if constexpr (__has_avx512bw_support_v<_ISA_>) return __as<_Intrin_>(_mm512_alignr_epi8(__as<__m512i>(__x), 
                _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 12), 16 - __shift_bytes));
            else {
                const auto __first = _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 12);

                const auto __low = _mm256_alignr_epi8(__as<__m256i>(__x), __as<__m256i>(__first), 16 - __shift_bytes);
                const auto __high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(__as<__m512i>(__x), 1), _mm512_extracti64x4_epi64(__first, 1), 16 - __shift_bytes);

                return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
            }
        }
        else if constexpr (__shift_bytes < 32) {
            if constexpr (__has_avx512bw_support_v<_ISA_>) return __as<_Intrin_>(_mm512_alignr_epi8(
                _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 12),
                _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 8), 32 - __shift_bytes));
            else {
                const auto __first = _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 12);
                const auto __second = _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 8);

                const auto __low = _mm256_alignr_epi8(__as<__m256i>(__first), __as<__m256i>(__second), 32 - __shift_bytes);
                const auto __high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(__first, 1), _mm512_extracti64x4_epi64(__second, 1), 32 - __shift_bytes);

                return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
            }
        }
        else if constexpr (__shift_bytes < 48) {
            if constexpr (__has_avx512bw_support_v<_ISA_>) return __as<_Intrin_>(_mm512_alignr_epi8(
                _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 8),
                _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 4), 48 - __shift_bytes));
            else {
                const auto __first = _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 8);
                const auto __second = _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 4);

                const auto __low = _mm256_alignr_epi8(__as<__m256i>(__first), __as<__m256i>(__second), 48 - __shift_bytes);
                const auto __high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(__first, 1), _mm512_extracti64x4_epi64(__second, 1), 48 - __shift_bytes);

                return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
            }
        }
        else if constexpr (__shift_bytes < 64) {
            if constexpr (__has_avx512bw_support_v<_ISA_>) return __as<_Intrin_>(_mm512_alignr_epi8(
                _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 4), __as<__m512i>(__y), 64 - __shift_bytes));
            else {
                const auto __first = _mm512_alignr_epi32(__as<__m512i>(__x), __as<__m512i>(__y), 4);

                const auto __low = _mm256_alignr_epi8(__as<__m256i>(__first), __as<__m256i>(__y), 64 - __shift_bytes);
                const auto __high = _mm256_alignr_epi8(_mm512_extracti64x4_epi64(__first, 1), _mm512_extracti64x4_epi64(__as<__m512i>(__y), 1), 64 - __shift_bytes);

                return __as<_Intrin_>(_mm512_inserti64x4(__as<__m512i>(__low), __high, 1));
            }
        }
    }
}

template <simd_type _Simd_, sizetype _Slide_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_right_merge(
    const _Simd_& __x, const _Simd_& __y, std::integral_constant<sizetype, _Slide_> __slide) noexcept
{
    using _Abi_ = abi_t<_Simd_>;
    using _Type_ = typename _Simd_::value_type;

    if constexpr (_Slide_ == 0) return __x;
    else if constexpr (_Slide_ == _Simd_::size()) return __y;
    else if constexpr (native<_Simd_>) {
        auto __r = __x;

        auto& __storage = __r.template __get<0>();
        __storage = __slide_right_merge_native<_Abi_::isa, _Type_>(__storage_unwrap(__x.template __get<0>()),
            __storage_unwrap(__y.template __get<0>()), __slide);

        return __r;
    }
    else return __slide_right_merge_fallback(__x, __y, __slide);
}

template <simd_type _Simd_>
raze_nodiscard raze_no_stack_protector raze_always_inline _Simd_ __slide_right_merge(
    const _Simd_& __x, const _Simd_& __y, i32 __sh) noexcept 
{
	return __slide_right_merge_fallback(__x, __y, __sh);
}

__RAZE_VX_NAMESPACE_END

#pragma strict_gs_check(on)