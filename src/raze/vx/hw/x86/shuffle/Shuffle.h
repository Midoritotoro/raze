#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/configurable/memory/Store.h>
#include <src/raze/vx/hw/configurable/memory/Load.h>

__RAZE_VX_NAMESPACE_BEGIN

template <
	class	_Type_, 
	u64	_Length_,
	u64	_Alignment_>
struct _Aligned_constexpr_array {
	alignas(_Alignment_) _Type_ __array[_Length_];
};

template <
	u64		_Alignment_, 
	u64 ...	_Indices_>
constexpr auto __expand_16bit_shuffle_to_8bit_shuffle(std::integer_sequence<u64, _Indices_...>) noexcept {
	constexpr auto __length = sizeof...(_Indices_);
	constexpr u64 __indices[] = { _Indices_... };

	_Aligned_constexpr_array<u8, __length << 1, _Alignment_> __result {};

	for (auto __i = 0; __i < __length; ++__i) {
		__result.__array[__i << 1] = static_cast<u8>(__indices[__i] << 1);
		__result.__array[(__i << 1) + 1] = static_cast<u8>((__indices[__i] << 1) + 1);
	}

	return __result;
}

template <
	u64		_Alignment_,
	u64 ...	_Indices_>
constexpr auto __expand_64bit_shuffle_to_32bit_shuffle(std::integer_sequence<u64, _Indices_...>) noexcept {
	constexpr auto __length = sizeof...(_Indices_);
	constexpr u64 __indices[] = { _Indices_... };

	_Aligned_constexpr_array<u32, __length << 1, _Alignment_> __result{};

	for (auto __i = 0; __i < __length; ++__i) {
		__result.__array[__i << 1] = static_cast<u32>(__indices[__i] << 1);
		__result.__array[(__i << 1) + 1] = static_cast<u32>((__indices[__i] << 1) + 1);
	}

	return __result;
}


template <
	arch::ISA _ISA_, class	_Type_,
	intrin_type _Intrin_, sizetype ...	_Indices_>
raze_nodiscard raze_always_inline _Intrin_ __shuffle_fallback(
	_Intrin_ __vector, std::integer_sequence<sizetype, _Indices_...>) noexcept
{
	constexpr auto __length = sizeof(_Intrin_) / sizeof(_Type_);
	using _IndexType = typename IntegerForSizeof<_Type_>::Unsigned;

	alignas(sizeof(_Intrin_)) constexpr _IndexType __indices_array[] = { _Indices_... };

	alignas(sizeof(_Intrin_)) _Type_ __values_array[__length];
	alignas(sizeof(_Intrin_)) _Type_ __result_array[__length];

	_Store<_ISA_>()(__values_array, __vector, __aligned_policy{});

	for (auto __i = 0; __i < __length; ++__i)
		__result_array[__i] = __values_array[__indices_array[__i]];

	return _Load<_ISA_, _Intrin_>()(__result_array, __aligned_policy{});
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Intrin_, class _Pattern_>
raze_always_inline _Intrin_ __shuffle_native(_Intrin_ __x, _Pattern_ __p) noexcept {
	alignas(64) static constexpr auto __indices = _Pattern_::template to_array<typename IntegerForSizeof<_Type_>::Unsigned>();
	alignas(64) static constexpr auto __expanded_16_8 = __expand_16bit_shuffle_to_8bit_shuffle<sizeof(_Intrin_)>(_Pattern_::get());
	alignas(64) static constexpr auto __expanded_64_32 = __expand_64bit_shuffle_to_32bit_shuffle<sizeof(_Intrin_)>(_Pattern_::get());

	static constexpr auto __ssse3 = __has_ssse3_support_v<_ISA_>;
	static constexpr auto __avx2 = __has_avx2_support_v<_ISA_>;
	static constexpr auto __avx512vl = __has_avx512vl_support_v<_ISA_>;
	static constexpr auto __avx512f = __has_avx512f_support_v<_ISA_>;
	static constexpr auto __avx512bw = __has_avx512bw_support_v<_ISA_>;
	static constexpr auto __avx512vbmi = __has_avx512vbmi_support_v<_ISA_>;

	if constexpr (sizeof(_Intrin_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			if constexpr (__indices[0] == 0) {
				if constexpr (__indices[1] == 0) return __as<_Intrin_>(_mm_unpacklo_pd(__as<__m128d>(__x), __as<__m128d>(__x)));
				else return __x;
			}
			else {
				if constexpr (__indices[1] == 0) return __as<_Intrin_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), 0x01));
				else return __as<_Intrin_>(_mm_unpackhi_pd(__as<__m128d>(__x), __as<__m128d>(__x)));
			}
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), __to_pshufd_mask<u8>(__p)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			if constexpr (!__across_halfs(__p)) {
				constexpr auto __low_shuf = (__indices[0] & 0x03) | ((__indices[1] & 0x03) << 2)
					| ((__indices[2] & 0x03) << 4) | ((__indices[3] & 0x03) << 6);

				constexpr auto __high_shuf = (__indices[4] & 0x03) | ((__indices[5] & 0x03) << 2)
					| ((__indices[6] & 0x03) << 4) | ((__indices[7] & 0x03) << 6);

				auto __shuffled = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_shufflehi_epi16(__shuffled, __high_shuf));
			}
			else if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x), _Load<_ISA_, __m128i>()(__expanded_16_8.__array, __aligned_policy{})));
			}
		}
		else if constexpr (sizeof(_Type_) == 1) {
			if constexpr (__can_widen_shuffle(__p) && !__across_halfs(__p)) {
				constexpr auto __widen = __widen_shuffle_pattern(__p);

				constexpr auto __low_shuf = (__widen[0] & 0x03) | ((__widen[1] & 0x03) << 2)
					| ((__widen[2] & 0x03) << 4) | ((__widen[3] & 0x03) << 6);

				constexpr auto __high_shuf = (__widen[4] & 0x03) | ((__widen[5] & 0x03) << 2)
					| ((__widen[6] & 0x03) << 4) | ((__widen[7] & 0x03) << 6);

				auto __shuffled = _mm_shufflelo_epi16(__as<__m128i>(__x), __low_shuf);
				return __as<_Intrin_>(_mm_shufflehi_epi16(__shuffled, __high_shuf));
			}
			else if constexpr (__ssse3) {
				return __as<_Intrin_>(_mm_shuffle_epi8(__as<__m128i>(__x),
					_Load<_ISA_, __m128i>()(__indices.data(), __aligned_policy{})));
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
		
	
	return __shuffle_fallback<_ISA_, _Type_>(__x, _Pattern_::get());
}

template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __shuffle_native_size(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept {
	using _Simd_ = typename _Pattern_::vector_type;
	using _Abi_ = typename _Simd_::abi_type;
	using _Value_ = typename _Simd_::value_type;

	_Simd_ __result = __x;
	__result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
		__chunk = __shuffle_native<_Abi_::isa, _Value_>(__storage_unwrap(__chunk), __p);
	});
	return __result;
}


template <class _Pattern_>
raze_always_inline typename _Pattern_::vector_type __shuffle(const typename _Pattern_::vector_type& __x, _Pattern_ __p) noexcept {
	using _Simd_ = typename _Pattern_::vector_type;
	using _Abi_ = typename _Simd_::abi_type;
	using _Value_ = typename _Simd_::value_type;
	
	if constexpr (__is_reverse(__p)) {
		if constexpr (sizeof(_Simd_::is_native())) {
			return __shuffle_native_size(__x, __p);
		}
		else {
			_Simd_ __result;

			[&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
				([&](auto __i) raze_always_inline_lambda {
					auto& __chunk1 = __x.template __get<__i>();
					auto& __chunk2 = __x.template __get<_Pattern_::size() - __i - 1>();
					
					std::remove_cvref_t<decltype(__chunk1)> __chunk1_temp = __chunk1;

					__chunk1 = __shuffle_native<_Abi_::isa, _Value_>(__storage_unwrap(__chunk2), make_reversed_pattern<typename decltype(__chunk2)::as_simd>{});
					__chunk2 = __shuffle_native<_Abi_::isa, _Value_>(__storage_unwrap(__chunk1_temp), make_reversed_pattern<typename decltype(__chunk1_temp)::as_simd>{});
				}(std::integral_constant<sizetype, _Indices_>{}), ...);
			}(std::make_integer_sequence<sizetype, _Pattern_::size()>{});

			return __result;
		}
	}
	
	return _Simd_{};
}

__RAZE_VX_NAMESPACE_END

