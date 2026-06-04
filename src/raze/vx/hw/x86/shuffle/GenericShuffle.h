#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>

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

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, class _Pattern_>
raze_always_inline _Intrin_ __generic_shuffle_native(_Intrin_ __x, _Pattern_ __p) noexcept {
	static constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512f = __has_avx512f_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx512vbmi = __has_avx512vbmi_support_v<_ISA_>;

#if __has_builtin(__builtin_shufflevector)
	return [&] <class _Type_, _Type_ ... __Is>(std::integer_sequence<_Type_, __Is...>) raze_always_inline_lambda {
		return __builtin_shufflevector(__x, __x, __Is...);
	}(__p.get());
#else

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

			if constexpr (!__across_halfs(__p)) {
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
		if constexpr (sizeof(_Type_) == 8 && __avx2) {
			return __as<_Intrin_>(_mm256_permute4x64_epi64(__as<__m256i>(__x), __to_pshufd_mask<u8>(__p)));
		}
		else if constexpr (sizeof(_Type_) == 4 && __avx2) {
			/*if constexpr (!__across_halfs(__p))
				return __as<_Intrin_>(_mm256_shuffle_ps(__as<__m256>(__x), __as<__m256>(__x), __p.template to_index_mask<u8>()));
			else if constexpr (__can_widen_shuffle(__p))
				return __as<_Intrin_>(_mm256_permute4x64_epi64(__as<__m256i>(__x), __widen_shuffle_pattern(__p)));
			else*/
				return __as<_Intrin_>(_mm256_permutevar8x32_epi32(__as<__m256i>(__x),
					_Load<_ISA_, __m256i>()(__indices.data(), __aligned_policy{})));
		}
		else if constexpr (sizeof(_Type_) == 2 && __avx2) {
			if constexpr (__avx512vbmi && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi8(_Load<_ISA_, __m256i>()(__expanded_16_8.__array, __aligned_policy{}), __as<__m256i>(__x)));
			}
			else if constexpr (__avx512bw && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi16(_Load<_ISA_, __m256i>()(__indices.data(), __aligned_policy{}), __as<__m256i>(__x)));
			}
			else if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_shuffle_epi8(__as<__m256i>(__x), _Load<_ISA_, __m256i>()(__expanded_16_8.__array, __aligned_policy{})));
			}
		}
		else if constexpr (sizeof(_Type_) == 1 && __avx2) {
			if constexpr (__avx512vbmi && __avx512vl) {
				return __as<_Intrin_>(_mm256_permutexvar_epi8(_Load<_ISA_, __m256i>()(__indices.data(), __aligned_policy{}), __as<__m256i>(__x)));
			}
			else if constexpr (!__across_halfs(__p)) {
				return __as<_Intrin_>(_mm256_shuffle_epi8(__as<__m256i>(__x), _Load<_ISA_, __m256i>()(__expanded_16_8.__array, __aligned_policy{})));
			}
		}
	}
	else if constexpr (sizeof(_Intrin_) == 64) {
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_Intrin_>(_mm512_permutevar_epi32(_Load<_ISA_, __m512i>()(
				__expanded_64_32.__array, __aligned_policy{}), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_Intrin_>(_mm512_permutevar_epi32(_Load<_ISA_, __m512i>()(
				__indices.data(), __aligned_policy{}), __as<__m512i>(__x)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			if constexpr (__avx512vbmi) {
				return __as<_Intrin_>(_mm512_permutexvar_epi8(_Load<_ISA_, __m512i>()(__expanded_16_8.__array, __aligned_policy{}), __as<__m512i>(__x)));
			}
			else if constexpr (__avx512bw) {
				return __as<_Intrin_>(_mm512_permutexvar_epi16(_Load<_ISA_, __m512i>()(__indices.data(), __aligned_policy{}), __as<__m512i>(__x)));
			}
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__avx512vbmi) {
				return __as<_Intrin_>(_mm512_permutexvar_epi8(_Load<_ISA_, __m512i>()(__indices.data(), __aligned_policy{}), __as<__m512i>(__x)));
			}
			else if constexpr (!__across_quads(__p) && __avx512bw) {
				return __as<_Intrin_>(_mm512_shuffle_epi8(__as<__m512i>(__x), _Load<_ISA_, __m512i>()(__expanded_16_8.__array, __aligned_policy{})));
			}
		}
	}
#endif // __has_builtin(__builtin_shufflevector)
	
	return __shuffle_fallback<_ISA_, _Type_>(__x, __p.get());
}

template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __generic_shuffle_native_size(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept {
	using _Simd_ = typename _Pattern_::vector_type;
	using _Abi_ = typename _Simd_::abi_type;
	using _Value_ = typename _Simd_::value_type;

	_Simd_ __result = __x;
	__result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
		__chunk = __generic_shuffle_native<_Abi_::isa, _Value_>(__storage_unwrap(__chunk), __p);
	});
	return __result;
}

template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __generic_shuffle(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept {
	using _Simd_ = typename _Pattern_::vector_type;
	using _Abi_ = typename _Simd_::abi_type;
	using _Value_ = typename _Simd_::value_type;

	if constexpr (__is_identity(__p)) {
		return __x;
	}
	else {

	}
}

__RAZE_VX_NAMESPACE_END 
