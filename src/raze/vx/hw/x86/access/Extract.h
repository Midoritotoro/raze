#pragma once 

#include <raze/memory/PointerToIntegral.h>
#include <src/raze/vx/hw/x86/memory/Load.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Element_, intrin_type _Intrin_>
raze_always_inline _Element_ __extract_first(_Intrin_ __x) noexcept {
	if constexpr (__is_epi64_v<_Element_> || __is_epu64_v<_Element_>) {
#if defined(raze_processor_x86_64)
		return _mm_cvtsi128_si64x(__as<__m128i>(__x));
#endif // defined(raze_processor_x86_64)
	}
	else if constexpr (__is_epi32_v<_Element_> || __is_epu32_v<_Element_>) {
		return _mm_cvtsi128_si32(__as<__m128i>(__x));
	}
	else if constexpr (__is_epi16_v<_Element_> || __is_epu16_v<_Element_>) {
		return _mm_cvtsi128_si32(__as<__m128i>(__x)) & 0xFFFF;
	}
	else if constexpr (__is_epi8_v<_Element_> || __is_epu8_v<_Element_>) {
		return _mm_cvtsi128_si32(__as<__m128i>(__x)) & 0xFF;
	}
	else if constexpr (__is_pd_v<_Element_>) {
		return _mm_cvtsd_f64(__as<__m128d>(__x));
	}
	else if constexpr (__is_ps_v<_Element_>) {
		return _mm_cvtss_f32(__as<__m128d>(__x));
	}
}

template <arch::ISA _ISA_, arithmetic_type _Element_>
struct _Extract {
    template <intrin_or_arithmetic_type _Tp_>
    raze_nodiscard raze_static_operator raze_always_inline _Element_ operator()(
        _Tp_ __vector, u8 __index) raze_const_operator noexcept
    {
        if constexpr (arithmetic_type<_Tp_>) {
            return __vector;
        }
        else {
            constexpr auto __length = sizeof(_Tp_) / sizeof(_Element_);

            alignas(sizeof(_Tp_)) _Element_ __array[__length];
            _Store<_ISA_>()(__array, __vector);

            return __array[__index & (__length - 1)];
        }
    }

	template <intrin_or_arithmetic_type _Tp_, sizetype _I_>
	raze_nodiscard raze_static_operator raze_always_inline _Element_ operator()(
		_Tp_ __x, std::integral_constant<sizetype, _I_> __index) raze_const_operator noexcept
	{
		if constexpr (arithmetic_type<_Tp_>) {
			return __x;
		}
		else if constexpr (__index == 0) {
			return __extract_first<_ISA_, _Element_>(__x);
		}
		else if constexpr (sizeof(_Tp_) == 16) {
			if constexpr (sizeof(_Element_) == 8 && std::is_integral_v<_Element_> && __has_sse41_support_v<_ISA_>) {
				return _mm_extract_epi64(__as<__m128i>(__x), __index);
			}
			else if constexpr (sizeof(_Element_) == 4 && __has_sse41_support_v<_ISA_>) {
				if constexpr (std::is_floating_point_v<_Element_>) return _mm_extract_ps(__as<__m128>(__x), __index);
				else return _mm_extract_epi32(__as<__m128i>(__x), __index);
			}
			else if constexpr (sizeof(_Element_) == 2) {
				return _mm_extract_epi16(__as<__m128i>(__x), __index);
			}
			else if constexpr (sizeof(_Element_) == 1) {
				return _mm_extract_epi8(__as<__m128i>(__x), __index);
			}
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (sizeof(_Element_) == 8 && std::is_integral_v<_Element_>) {
				return _mm256_extract_epi64(__as<__m256i>(__x), __index);
			}
			else if constexpr (sizeof(_Element_) == 4 && std::is_integral_v<_Element_>) {
				return _mm256_extract_epi32(__as<__m256i>(__x), __index);
			}
			else if constexpr (sizeof(_Element_) == 2 && __has_avx2_support_v<_ISA_>) {
				return _mm256_extract_epi16(__as<__m256i>(__x), __index);
			}
			else if constexpr (sizeof(_Element_) == 1 && __has_avx2_support_v<_ISA_>) {
				return _mm256_extract_epi8(__as<__m256i>(__x), __index);
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_pd_v<_Element_>) {
				return _mm512_cvtsd_f64(_mm512_maskz_compress_pd(
					static_cast<u8>(u8(1) << __index), __as<__m512d>(__x)));
			}
			else if constexpr (__is_ps_v<_Element_>) {
				return _mm512_cvtss_f32(_mm512_maskz_compress_ps(
					static_cast<u8>(u8(1) << __index), __as<__m512>(__x)));
			}
			else if constexpr (__is_epi32_v<_Element_> || __is_epu32_v<_Element_>) {
				return _mm_cvtsi128_si32(__as<__m128i>(_mm512_maskz_compress_epi32(
					static_cast<u8>(u8(1) << __index), __as<__m512i>(__x))));
			}
			else if constexpr (__is_epi64_v<_Element_> || __is_epu64_v<_Element_>) {
				return _mm_cvtsi128_si64(__as<__m128i>(_mm512_maskz_compress_epi64(
					static_cast<u8>(u8(1) << __index), __as<__m512i>(__x))));
			}
		}

		return _Extract()(__x, __index.value);
	}
};

template <intrin_type _ExtractVector_, arch::ISA _ISA_>
struct _Extract_vector {
	template <intrin_type _Tp_, sizetype _I_>
	raze_static_operator raze_always_inline _ExtractVector_ operator()(_Tp_ __vector, std::integral_constant<sizetype, _I_> __index) raze_const_operator noexcept {
		if constexpr (sizeof(_Tp_) == sizeof(_ExtractVector_)) {
			return __as<_Tp_>(__vector);
		}
		else if constexpr (sizeof(_Tp_) == 32) {
			if constexpr (sizeof(_Tp_) == 16) {
				return _mm256_extracti128_si256(__as<__m256i>(__vector), __index);
			}
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (sizeof(_Tp_) == 16) {
				__vector = _mm512_extracti64x2_epi64(__as<__m512i>(__vector), __index);
			}
			else if constexpr (sizeof(_Tp_) == 32) {
				__vector = _mm512_extracti64x4_epi64(__as<__m512i>(__vector), __index);
			}
		}
	}
};

__RAZE_VX_NAMESPACE_END
