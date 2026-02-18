#pragma once 

#include <src/raze/datapar/arithmetic/Sub.h>
#include <src/raze/datapar/shuffle/Broadcast.h>

#include <raze/memory/PointerToIntegral.h>
#include <src/raze/datapar/shuffle/InsertMask.h>

#include <src/raze/datapar/memory/Load.h>
#include <src/raze/datapar/shuffle/Blend.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
    arch::ISA	_ISA_,
    uint32		_Width_,
    class       _DesiredType_>
struct _Simd_extract;

template <class _DesiredType_>
struct _Simd_extract<arch::ISA::SSE2, 128, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_&   __vector,
        uint8			__index) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
            if (__index == 0) {
#if defined(raze_processor_x86_64)
                return static_cast<_DesiredType_>(_mm_cvtsi128_si64(__intrin_bitcast<__m128i>(__vector)));
#else
                const auto __high_dword = _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__vector));
                const auto __low_dword = _mm_cvtsi128_si32(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0x55));

                return (static_cast<int64>(__high_dword) << 32) | static_cast<int64>(__low_dword);
#endif // defined(raze_processor_x86_64)
            }

#if defined(raze_processor_x86_64)
            const auto __shuffled = _mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0xEE);
            return static_cast<_DesiredType_>(_mm_cvtsi128_si64(__shuffled));
#else
            const auto __high_dword = _mm_cvtsi128_si32(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0xEE));
            const auto __low_dword = _mm_cvtsi128_si32(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0xFF));

            return (static_cast<int64>(__high_dword) << 32) | static_cast<int64>(__low_dword);
#endif // defined(raze_processor_x86_64)
        }
        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm_cvtsi128_si32(__intrin_bitcast<__m128i>(__vector)));

                case 1:
                    return static_cast<_DesiredType_>(_mm_cvtsi128_si32(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0x55)));

                case 2:
                    return static_cast<_DesiredType_>(_mm_cvtsi128_si32(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0xEE)));

                default:
                    return static_cast<_DesiredType_>(_mm_cvtsi128_si32(_mm_shuffle_epi32(__intrin_bitcast<__m128i>(__vector), 0xFF)));
            }
        }
        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 0));

                case 1:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 1));

                case 2:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 2));

                case 3:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 3));

                case 4:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 4));

                case 5:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 5));

                case 6:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 6));

                default:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 7));
            }
        }
        else {
            constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);
            alignas(16) _DesiredType_ __array[__length];

            _mm_store_si128(reinterpret_cast<__m128i*>(&__array), __intrin_bitcast<__m128i>(__vector));
            return __array[__index & (__length - 1)];
        }
    }
};

template <class _DesiredType_>
struct _Simd_extract<arch::ISA::SSE41, 128, _DesiredType_> :
    _Simd_extract<arch::ISA::SSSE3, 128, _DesiredType_>
{
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_&   __vector,
        uint8			__index) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm_extract_epi64(__intrin_bitcast<__m128i>(__vector), 0));

                default:
                    return static_cast<_DesiredType_>(_mm_extract_epi64(__intrin_bitcast<__m128i>(__vector), 1));
            }
        }
        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm_extract_epi32(__intrin_bitcast<__m128i>(__vector), 0));

                case 1:
                    return static_cast<_DesiredType_>(_mm_extract_epi32(__intrin_bitcast<__m128i>(__vector), 1));

                case 2:
                    return static_cast<_DesiredType_>(_mm_extract_epi32(__intrin_bitcast<__m128i>(__vector), 2));

                default:
                    return static_cast<_DesiredType_>(_mm_extract_epi32(__intrin_bitcast<__m128i>(__vector), 3));
            }
        }
        else if constexpr (__is_epi16_v<_DesiredType_> || __is_epu16_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 0));

                case 1:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 1));

                case 2:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 2));

                case 3:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 3));

                case 4:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 4));

                case 5:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 5));

                case 6:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 6));

                default:
                    return static_cast<_DesiredType_>(_mm_extract_epi16(__intrin_bitcast<__m128i>(__vector), 7));
            }
        }
        else {
            return _Simd_extract<arch::ISA::SSE2, 128, _DesiredType_>()(__vector, __index);
        }
    }
};


template <class _DesiredType_>
struct _Simd_extract<arch::ISA::AVX2, 256, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_& __vector,
        uint8			__index) raze_const_operator noexcept
    {
        if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm256_extract_epi64(__intrin_bitcast<__m256i>(__vector), 0));

                case 1:
                    return static_cast<_DesiredType_>(_mm256_extract_epi64(__intrin_bitcast<__m256i>(__vector), 1));

                case 2:
                    return static_cast<_DesiredType_>(_mm256_extract_epi64(__intrin_bitcast<__m256i>(__vector), 2));

                case 3:
                    return static_cast<_DesiredType_>(_mm256_extract_epi64(__intrin_bitcast<__m256i>(__vector), 3));
            }
        }
        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            switch (__index) {
                case 0:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 0));

                case 1:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 1));

                case 2:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 2));

                case 3:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 3));

                case 4:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 4));

                case 5:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 5));

                case 6:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 6));

                case 7:
                    return static_cast<_DesiredType_>(_mm256_extract_epi32(__intrin_bitcast<__m256i>(__vector), 7));
            }
        }
        else {
            constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);
            alignas(32) _DesiredType_ __array[__length];

            _mm256_store_si256(reinterpret_cast<__m256i*>(&__array), __intrin_bitcast<__m256i>(__vector));
            return __array[__index & (__length - 1)];
        }
    }
};

template <class _DesiredType_>
struct _Simd_extract<arch::ISA::AVX512F, 512, _DesiredType_> {
    template <class _IntrinType_>
    raze_nodiscard raze_static_operator raze_always_inline _DesiredType_ operator()(
        _IntrinType_&   __vector,
        uint8			__index) raze_const_operator noexcept
    {
        if constexpr (__is_pd_v<_DesiredType_>) {
            return _mm512_cvtsd_f64(_mm512_maskz_compress_pd(
                static_cast<uint8>(uint8(1) << __index), __intrin_bitcast<__m512d>(__vector)));
        }
        else if constexpr (__is_ps_v<_DesiredType_>) {
            return _mm512_cvtss_f32(_mm512_maskz_compress_ps(
                static_cast<uint16>(uint16(1) << __index), __intrin_bitcast<__m512>(__vector)));
        }
        else if constexpr (__is_epi32_v<_DesiredType_> || __is_epu32_v<_DesiredType_>) {
            return _mm_cvtsi128_si32(__intrin_bitcast<__m128i>(_mm512_maskz_compress_epi32(
                static_cast<uint16>(uint16(1) << __index), __intrin_bitcast<__m512i>(__vector))));
        }
        else if constexpr (__is_epi64_v<_DesiredType_> || __is_epu64_v<_DesiredType_>) {
            return _mm_cvtsi128_si64(__intrin_bitcast<__m128i>(_mm512_maskz_compress_epi64(
                static_cast<uint8>(uint8(1) << __index), __intrin_bitcast<__m512i>(__vector))));
        }
        else {
            constexpr auto __length = sizeof(_IntrinType_) / sizeof(_DesiredType_);
            alignas(64) _DesiredType_ __array[__length];

            _mm512_store_si512(__array, __vector);
            return __array[__index & (__length - 1)];
        }
    }
};

template <class _DesiredType_> struct _Simd_extract<arch::ISA::SSE3, 128, _DesiredType_> : _Simd_extract<arch::ISA::SSE2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::SSSE3, 128, _DesiredType_> : _Simd_extract<arch::ISA::SSE3, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::SSE42, 128, _DesiredType_> : _Simd_extract<arch::ISA::SSE41, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX2, 128, _DesiredType_> : _Simd_extract<arch::ISA::SSE42, 128, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512BW, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512DQ, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512F, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512BWDQ, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512BW, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI2, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VBMI, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512BWDQ, 512, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI2DQ, 512, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VBMIDQ, 512, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLF, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX2, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLBW, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLDQ, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLF, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLBW, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI2VL, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VBMIVL, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLBWDQ, 256, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI2VLDQ, 256, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VBMIVLDQ, 256, _DesiredType_> {};

template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLF, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX2, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLBW, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLDQ, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLF, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLBW, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI2VL, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VBMIVL, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VLBWDQ, 128, _DesiredType_> {};
template <class _DesiredType_> struct _Simd_extract<arch::ISA::AVX512VBMI2VLDQ, 128, _DesiredType_> : _Simd_extract<arch::ISA::AVX512VBMIVLDQ, 128, _DesiredType_> {};

__RAZE_DATAPAR_NAMESPACE_END
