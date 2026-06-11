#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/access/Extract.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>
#include <src/raze/vx/hw/x86/access/Extract.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Tp_>
raze_nodiscard raze_always_inline auto __zmm_broadcast_low(_Tp_ __x) noexcept {
	if constexpr (sizeof(_Type_) == 8) return _mm512_broadcastq_epi64(__as<__m128i>(__x));
	else if constexpr (sizeof(_Type_) == 4) return _mm512_broadcastd_epi32(__as<__m128i>(__x));
	else if constexpr (sizeof(_Type_) == 2 && __has_avx512bw_support_v<_ISA_>) return _mm512_broadcastw_epi16(__as<__m128i>(__x));
	else if constexpr (sizeof(_Type_) == 1 && __has_avx512bw_support_v<_ISA_>) return _mm512_broadcastb_epi8(__as<__m128i>(__x));
	return _Broadcast<_ISA_, __m512i>()(_Extract<_ISA_, _Type_>()(__x, std::integral_constant<sizetype, 0>{}));
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Tp_, sizetype _I_>
raze_nodiscard raze_always_inline _Tp_ __splat_native(_Tp_ __x, std::integral_constant<sizetype, _I_> __i) noexcept
	requires(__i >= 0 && __i < (sizeof(_Tp_) / sizeof(_Type_)))
{
	constexpr auto __size = sizeof(_Tp_) / sizeof(_Type_);

	if constexpr (sizeof(_Tp_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			if constexpr (__i == 0) return __as<_Tp_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), 0));
			else return __as<_Tp_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), 0x03));
		}
		else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm_shuffle_epi32(__as<__m128i>(__x), __broadcast_pshufd_index(__i)));
		else if constexpr (sizeof(_Type_) == 2 && __has_avx2_support_v<_ISA_> && __i == 0) return __as<_Tp_>(_mm_broadcastw_epi16(__as<__m128i>(__x)));
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__has_avx2_support_v<_ISA_> && __i == 0) return __as<_Tp_>(_mm_broadcastb_epi8(__as<__m128i>(__x)));
			else if constexpr (!__has_ssse3_support_v<_ISA_>) {
				__m128i __combined;

				if constexpr (__i < 8) __combined = _mm_unpacklo_epi8(__as<__m128i>(__x), __as<__m128i>(__x));
				else __combined = _mm_unpackhi_epi8(__as<__m128i>(__x), __as<__m128i>(__x));

				return __as<_Tp_>(__generic_shuffle_native<_ISA_, i16>(__combined, make_splat_pattern<simd<i16, runtime_abi<_ISA_, 8>>, __i % 8>{}));
			}
		}
		return __generic_shuffle_native<_ISA_, _Type_>(__x, make_splat_pattern<simd<_Type_, runtime_abi<_ISA_, __size>>, __i>{});
	}
	else if constexpr (sizeof(_Tp_) == 32) {
		constexpr auto __index = std::integral_constant<sizetype, __i % (__size / 2)>{};

		if constexpr (__i == 0 && __has_avx2_support_v<_ISA_>) {
			if constexpr (sizeof(_Type_) == 8)		return __as<_Tp_>(_mm256_broadcastq_epi64(__as<__m128i>(__x)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm256_broadcastd_epi32(__as<__m128i>(__x)));
			else if constexpr (sizeof(_Type_) == 2) return __as<_Tp_>(_mm256_broadcastw_epi16(__as<__m128i>(__x)));
			else if constexpr (sizeof(_Type_) == 1) return __as<_Tp_>(_mm256_broadcastb_epi8(__as<__m128i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 8) return __generic_shuffle_native<_ISA_, _Type_, _Tp_>(__x,
			make_splat_pattern<simd<_Type_, runtime_abi<_ISA_, 4>>, __i>{});
		else if constexpr (sizeof(_Type_) == 4) {
			constexpr auto __index = std::integral_constant<sizetype, __i % (__size / 2)>{};

			if constexpr (__i < (__size / 2)) {
				const auto __half = __splat_native<_ISA_, _Type_>(__as<__m128i>(__x), __index);
				return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__half), __as<__m128i>(__half), 1));
			}
			else {
				const auto __half = _mm256_permute_ps(__as<__m256>(__x), __broadcast_pshufd_index(std::integral_constant<sizetype, __i % (__size / 2)>{}));
				return __as<_Tp_>(_mm256_permute2f128_si256(__as<__m256i>(__half), __as<__m256i>(__half), 0x31));
			}
		}
		else {
			__m128i __half;

			if constexpr (__i < (__size / 2)) __half = __splat_native<_ISA_, _Type_>(__as<__m128i>(__x), __index);
			else __half = __splat_native<_ISA_, _Type_>(_mm256_extractf128_si256(__as<__m256i>(__x), 1), __index);

			return __as<_Tp_>(_mm256_insertf128_si256(__as<__m256i>(__half), __as<__m128i>(__half), 1));
		}
	}
	else if constexpr (sizeof(_Tp_) == 64) {
		if constexpr (__i == 0) return __as<_Tp_>(__zmm_broadcast_low<_ISA_, _Type_>(__x));
		else {
			constexpr auto __index = std::integral_constant<sizetype, __i % (__size / 4)>{};
			constexpr auto __lane = __i / (__size / 4);

			return __as<_Tp_>(__zmm_broadcast_low<_ISA_, _Type_>(__splat_native<_ISA_, _Type_>(
				_mm512_extracti32x4_epi32(__as<__m512i>(__x), __lane), __index)));
		}
	}
}

template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __splat(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Tp_ = pattern_vector_t<_Pattern_>;

	if constexpr (native<_Tp_>) {
		using _Ret = decltype(__splat_native<abi_t<_Tp_>::isa, typename _Tp_::value_type>(__storage_unwrap(__x.template __get<0>()), __p.at<0>()));

		if constexpr (!std::is_void_v<_Ret>) {
			auto __r = __x;

			auto& __storage = __r.template __get<0>();
			__storage = __splat_native<abi_t<_Tp_>::isa, typename _Tp_::value_type>(__storage_unwrap(__storage), __p.at<0>());

			return __r;
		}
		else return __generic_shuffle(__x, __p);
	}

	return __x[__p.at<0>()];
}

__RAZE_VX_NAMESPACE_END
