#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, class _Type_,
	intrin_type _Intrin_, sizetype ...	_Indices_>
raze_nodiscard raze_always_inline _Intrin_ __shuffle_fallback(
	_Intrin_ __vector, std::integer_sequence<sizetype, _Indices_...>) noexcept
{
	constexpr auto __length = sizeof(_Intrin_) / sizeof(_Type_);
	using _IndexType = typename IntegerForSizeof<_Type_>::Unsigned;

	alignas(sizeof(_Intrin_)) static constexpr _IndexType __indices_array[] = { _Indices_... };

	alignas(sizeof(_Intrin_)) _Type_ __values_array[__length];
	alignas(sizeof(_Intrin_)) _Type_ __result_array[__length];

	_Store<_ISA_>()(__values_array, __vector, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__result_array[__i] = __values_array[__indices_array[__i]];

	return _Load<_ISA_, _Intrin_>()(__result_array, __aligned_policy{});
}

template <class _Pattern_>
raze_always_inline pattern_vector_t<_Pattern_> __generic_shuffle_scalar_fallback(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
#if 0
	pattern_vector_t<_Pattern_> __temp = __x;
	
	[&] <sizetype ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
		([&] (auto __i) raze_always_inline_lambda {
			__temp[__i] = __x[__p.at<__i>()];
		}(std::integral_constant<sizetype, __I>{}), ...);
	}(__p.get());

	return __temp;
#else
	using _Simd_ = pattern_vector_t<_Pattern_>;

	alignas(64) typename _Simd_::value_type __dst[_Simd_::size()];
	alignas(64) typename _Simd_::value_type __src[_Simd_::size()];

	vx::__store[vx::aligned](__dst, __x);
	vx::__store[vx::aligned](__src, __x);

	[&] <sizetype ... __I> (std::integer_sequence<sizetype, __I...>) raze_always_inline_lambda {
		([&](auto __i) raze_always_inline_lambda{
			__dst[__i] = __src[__p.at<__i>()];
		}(std::integral_constant<sizetype, __I>{}), ...);
	}(__p.get());

	return vx::__load<_Simd_>[vx::aligned](__dst);
#endif
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, class _Pattern_>
raze_always_inline _Intrin_ __generic_shuffle_native(_Intrin_ __x, _Pattern_ __p) noexcept {
	static constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512f = __has_avx512f_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx512vbmi = __has_avx512vbmi_support_v<_ISA_>;

	if constexpr (sizeof(_Intrin_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __shufpd_to_pshufd_mask(__p)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __to_pshufd_mask(__p)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			constexpr auto __low_shuf = __to_pshufd_mask(__p);
			constexpr auto __high_shuf = (__p[4] & 0x03) | ((__p[5] & 0x03) << 2) | ((__p[6] & 0x03) << 4) | ((__p[7] & 0x03) << 6);

			if constexpr (__can_widen_shuffle(__p)) {
				return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __to_pshufd_mask(__p.widen())));
			}
			else if constexpr (!__across_halfs(__p)) {
				auto __shuffled = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_shufflehi_epi16(__shuffled, __high_shuf));
			}
			else if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __p.template expand<u16, u8>().template as_native<__m128i>()));
			}
			else if constexpr (__is_dup_low(__p)) {
				const auto __low = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_unpacklo_epi64(__low, __low));
			}
			else if constexpr (__is_dup_high(__p)) {
				const auto __high = _mm_shufflehi_epi16(__as<__m128i>(__x), __high_shuf);
				return __as<_Intrin_>(_mm_unpackhi_epi64(__high, __high));
			}
			else if constexpr (__is_low_half(__p)) {
				const auto __low1 = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __low2 = _mm_shufflelo_epi16(__as<__m128i>(__x), __high_shuf);
				return __as<_Intrin_>(_mm_unpacklo_epi64(__low1, __low2));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __high1 = _mm_shufflehi_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __high2 = _mm_shufflehi_epi16(__as<__m128i>(__x), __high_shuf);
				return __as<_Intrin_>(_mm_unpacklo_epi64(__high1, __high2));
			}
			else {
				const auto __low1 = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __low2 = _mm_shufflelo_epi16(__as<__m128i>(__x), __high_shuf);
				const auto __high1 = _mm_shufflehi_epi16(__as<__m128i>(__x), __low_shuf);
				const auto __high2 = _mm_shufflehi_epi16(__as<__m128i>(__x), __high_shuf);

				const auto __low = _mm_unpacklo_epi64(__low1, __low2);
				const auto __high = _mm_unpackhi_epi64(__high1, __high2);

				return _Select<_ISA_, _Type_>()(__low, __high, _To_mask<_ISA_, _Type_>()(__p < std::integral_constant<sizetype, 4>{}));
			}
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__can_widen_shuffle(__p) && !__across_halfs(__p)) {
				constexpr auto __widen = __p.widen();

				constexpr auto __low_shuf = __to_pshufd_mask(__widen);
				constexpr auto __high_shuf = (__widen[4] & 0x03) | ((__widen[5] & 0x03) << 2)
					| ((__widen[6] & 0x03) << 4) | ((__widen[7] & 0x03) << 6);

				auto __shuffled = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_shufflehi_epi16(__shuffled, __high_shuf));
			}
			else if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), __p.template as_native<__m128i>()));
			}
		}
	}
	else if constexpr (sizeof(_Intrin_) == 32) {
		if constexpr (sizeof(_Type_) == 8) {
			constexpr auto __mask = ((__p[0] % 2)) | ((__p[1] % 2) << 1) | ((__p[2] % 2) << 2) | ((__p[3] % 2) << 3);

			if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_permute_pd(__as<__m256d>(__x), __mask));
			}
			else if constexpr (__avx2) return __as<_Intrin_>(_mm256_permute4x64_epi64(__as<__m256i>(__x), __to_pshufd_mask(__p)));
			else if constexpr (__is_low_half(__p)) {
				const auto __broadcasted_low_lane = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 0);
				return __as<_Intrin_>(_mm256_permute_pd(__broadcasted_low_lane, __mask));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __broadcasted_high_lane = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 0x11);
				return __as<_Intrin_>(_mm256_permute_pd(__broadcasted_high_lane, __mask));
			}
			else {
				constexpr auto __blend_mask = (__p / std::integral_constant<sizetype, 2>{}) != _Shuffle_pattern<pattern_vector_t<_Pattern_>, 0, 0, 1, 1>{};
				const auto __swapped_lanes = _mm256_permute2f128_pd(__as<__m256d>(__x), __as<__m256d>(__x), 0x01);

				const auto __first = _mm256_permute_pd(__as<__m256d>(__x), __mask);
				const auto __second = _mm256_permute_pd(__swapped_lanes, __mask);
				
				return __as<_Intrin_>(_mm256_blend_pd(__first, __second, __blend_mask));
			}
		}
		else if constexpr (sizeof(_Type_) == 4) {
			if constexpr (!__across_halfs(__p))
				return __as<_Intrin_>(_mm256_shuffle_ps(__as<__m256>(__x), __as<__m256>(__x), __to_pshufd_mask(__p)));
			else if constexpr (__can_widen_shuffle(__p) && __avx2)
				return __as<_Intrin_>(_mm256_permute4x64_epi64(__as<__m256i>(__x), __to_pshufd_mask(__p.widen())));
			else if constexpr (__avx2)
				return __as<_Intrin_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__x), __p.template as_native<__m256i>()));
			else if constexpr (__is_dup_low(__p)) {
				auto __split = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0);
				
				if constexpr (!__is_dup_low_identity(__p))
					__split = _mm256_permute_ps(__split, __to_pshufd_mask(__p));

				return __as<_Intrin_>(__split);
			}
			else if constexpr (__is_dup_high(__p)) {
				constexpr auto __mask = (((__p[4] - 4) & 0x03) | (((__p[5] - 4) & 0x03) << 2)
					| (((__p[6] - 4) & 0x03) << 4) | (((__p[7] - 4) & 0x03) << 6));

				auto __split = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0x11);

				if constexpr (!__is_dup_low_identity(__p))
					__split = _mm256_permute_ps(__split, __mask);

				return __as<_Intrin_>(__split);
			}
			else if constexpr (__is_low_half(__p)) {
				const auto __broadcasted_low_lane = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0);
				return __as<_Intrin_>(_mm256_permutevar_ps(__broadcasted_low_lane, (__p % std::integral_constant<sizetype, 4>{}).template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __broadcasted_low_lane = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0x11);
				return __as<_Intrin_>(_mm256_permutevar_ps(__broadcasted_low_lane, (__p % std::integral_constant<sizetype, 4>{}).template as_native<__m256i>()));
			}
			else {
				constexpr auto __low_mask = (__p % std::integral_constant<sizetype, 4>{});
				constexpr auto __blend_mask = (__p / std::integral_constant<sizetype, 4>{}) != _Shuffle_pattern<pattern_vector_t<_Pattern_>, 0, 0, 0, 0, 1, 1, 1, 1>{};
				const auto __swapped_lanes = _mm256_permute2f128_ps(__as<__m256>(__x), __as<__m256>(__x), 0x01);

				const auto __native_mask = __p.template as_native<__m256i>();

				const auto __first = _mm256_permutevar_ps(__as<__m256>(__x), __native_mask);
				const auto __second = _mm256_permutevar_ps(__swapped_lanes, __native_mask);

				return __as<_Intrin_>(_mm256_blend_ps(__first, __second, __blend_mask));
			}
		}
		else if constexpr (sizeof(_Type_) == 2 && __avx2) {
			constexpr auto __expanded = __p.expand<u16, u8>();
			constexpr auto __mask = __expanded % std::integral_constant<sizetype, 16>{};

			if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_shuffle_epi8(__as<__m256i>(__x), __p.expand<u16, u8>.template as_native<__m256i>()));
			}
			else if constexpr (__avx512bw && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi16(__p.template as_native<__m256i>(), __as<__m256i>(__x)));
			}
			else if constexpr (__is_low_half(__p)) { 
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) { 
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x11);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else {
				constexpr auto __expanded = __p.expand<u16, u8>();
				const auto __swapped = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x01);

				const auto __shuffled1 = _mm256_shuffle_epi8(__as<__m256i>(__x), __expanded.template as_native<__m256i>());
				const auto __shuffled2 = _mm256_shuffle_epi8(__swapped, __mask.template as_native<__m256i>());

				return __as<_Intrin_>(_mm256_or_si256(__shuffled1, __shuffled2));
			}
		}
		else if constexpr (sizeof(_Type_) == 1 && __avx2) {
			constexpr auto __mask = (__p % std::integral_constant<sizetype, 16>{});

			if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_shuffle_epi8(__as<__m256i>(__x), __p));
			}
			else if constexpr (__avx512vbmi && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi8(__p.template as_native<__m256i>(), __as<__m256i>(__x)));
			}
			else if constexpr (__is_low_half(__p)) {
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else if constexpr (__is_high_half(__p)) {
				const auto __broadcasted_low = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x11);
				return __as<_Intrin_>(_mm256_shuffle_epi8(__broadcasted_low, __mask.template as_native<__m256i>()));
			}
			else {
				const auto __swapped = _mm256_permute2x128_si256(__as<__m256i>(__x), __as<__m256i>(__x), 0x01);

				const auto __shuffled1 = _mm256_shuffle_epi8(__as<__m256i>(__x), __p.template as_native<__m256i>());
				const auto __shuffled2 = _mm256_shuffle_epi8(__swapped, __mask.template as_native<__m256i>());

				return __as<_Intrin_>(_mm256_or_si256(__shuffled1, __shuffled2));
			}
		}
	}
	else if constexpr (sizeof(_Intrin_) == 64) {
		if constexpr (sizeof(_Type_) == 8) {
			if constexpr (!__across_quads(__p)) {
				constexpr auto __mask = ((__p[0] & 1) << 0) | ((__p[1] & 1) << 1) | ((__p[2] & 1) << 2) | ((__p[3] & 1) << 3) |
					((__p[4] & 1) << 4) | ((__p[5] & 1) << 5) | ((__p[6] & 1) << 6) | ((__p[7] & 1) << 7);

				return __as<_Intrin_>(_mm512_permute_pd(__as<__m512d>(__x), __mask));
			}
			return __as<_Intrin_>(_mm512_permutexvar_epi64(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			if constexpr (!__across_quads(__p)) {
				constexpr auto __q0 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 0>{}));
				constexpr auto __q1 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 4>{}));
				constexpr auto __q2 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 8>{}));
				constexpr auto __q3 = __to_pshufd_mask(__p.offset(std::integral_constant<sizetype, 12>{}));

				if constexpr (__q0 == __q1 && __q1 == __q2 && __q2 == __q3)
					return __as<_Intrin_>(_mm512_permute_ps(__as<__m512>(__x), __q0));
				else
					return __as<_Intrin_>(_mm512_permutevar_ps(__as<__m512>(__x), __p.template as_native<__m512i>()));
			}
			else return __as<_Intrin_>(_mm512_permutevar_epi32(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			if constexpr (__avx512bw) return __as<_Intrin_>(_mm512_permutexvar_epi16(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (!__across_quads(__p) && __avx512bw)
				return __as<_Intrin_>(_mm512_shuffle_epi8(__as<__m512i>(__x), __p.template as_native<__m512i>()));
			else if constexpr (__avx512vbmi)
				return __as<_Intrin_>(_mm512_permutexvar_epi8(__p.template as_native<__m512i>(), __as<__m512i>(__x)));
		}
	}

#if __has_builtin(__builtin_shufflevector)
	return[&] <class _Type_, _Type_ ... __Is>(std::integer_sequence<_Type_, __Is...>) raze_always_inline_lambda {
		return __builtin_shufflevector(__x, __x, __Is...);
	}(__p.get());
#else
	return __shuffle_fallback<_ISA_, _Type_>(__x, __p.get());
#endif // __has_builtin(__builtin_shufflevector)
}

template <class _Pattern_>
raze_always_inline pattern_vector_t<_Pattern_> __generic_shuffle_native_size(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Simd_ = pattern_vector_t<_Pattern_>;

	_Simd_ __result = __x;
	__result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
		__chunk = __generic_shuffle_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__chunk), __p);
	});

	return __result;
}

template <class _Pattern_>
raze_always_inline pattern_vector_t<_Pattern_> __generic_shuffle(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	if constexpr (__is_identity(__p)) return __x;
	else if constexpr (pattern_vector_t<_Pattern_>::is_native()) return __generic_shuffle_native_size(__x, __p);
	else return __generic_shuffle_scalar_fallback(__x, __p);
}

__RAZE_VX_NAMESPACE_END 
