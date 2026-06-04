#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Value_, intrin_type _Intrin_, class _Pattern_>
raze_always_inline _Intrin_ __reverse_native(_Intrin_ __x, _Pattern_ __p) noexcept {
	if constexpr (!__has_ssse3_support_v<_ISA_> && sizeof(_Intrin_) == 16) {
		if constexpr (sizeof(_Value_) == 2) {
			__x = __as<_Intrin_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), 1));
			__x = __as<_Intrin_>(_mm_shufflehi_epi16(__as<__m128i>(__x), 0x1B));
			return __as<_Intrin_>(_mm_shufflelo_epi16(__as<__m128i>(__x), 0x1B));
		}
		else if constexpr (sizeof(_Value_) == 1) {
			__x = __as<_Intrin_>(_mm_or_si128(_mm_srli_epi16(__as<__m128i>(__x), 8), _mm_slli_epi16(__as<__m128i>(__x), 8)));
			__x = __as<_Intrin_>(_mm_shufflelo_epi16(__as<__m128i>(__x), 0x1B));
			__x = __as<_Intrin_>(_mm_shufflehi_epi16(__as<__m128i>(__x), 0x1B));
			return __as<_Intrin_>(_mm_shuffle_epi32(__as<__m128i>(__x), 0x4E));
		}
	}

	return __generic_shuffle_native<_ISA_, _Value_>(__x, __p);
}

template <class _Pattern_>
raze_always_inline pattern_vector_t<_Pattern_> __reverse_native_size(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Simd_ = pattern_vector_t<_Pattern_>;

	_Simd_ __result = __x;
	__result.__for_each_chunk([&] <class _Chunk> (_Chunk& __chunk) raze_always_inline_lambda {
		__chunk = __reverse_native<abi_t<_Simd_>::isa, typename _Simd_::value_type>(__storage_unwrap(__chunk), __p);
	});

	return __result;
}

template <class _Pattern_>
raze_nodiscard raze_always_inline pattern_vector_t<_Pattern_> __reverse(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Simd_ = pattern_vector_t<_Pattern_>;
	using _Value_ = typename _Simd_::value_type;

	if constexpr (_Simd_::is_native()) {
		return __reverse_native_size(__x, _Pattern_{});
	}
	else {
		_Simd_ __result;

		[&] <sizetype ... _Indices_> (std::integer_sequence<sizetype, _Indices_...>) raze_always_inline_lambda {
			([&](auto __i) raze_always_inline_lambda {
				auto& __chunk1 = __x.template __get<__i>();
				auto& __chunk2 = __x.template __get<_Pattern_::size() - __i - 1>();

				std::remove_cvref_t<decltype(__chunk1)> __chunk1_temp = __chunk1;

				__chunk1 = __reverse_native<abi_t<_Simd_>::isa, _Value_>(__storage_unwrap(__chunk2), make_reversed_pattern<typename decltype(__chunk2)::as_simd>{});
				__chunk2 = __reverse_native<abi_t<_Simd_>::isa, _Value_>(__storage_unwrap(__chunk1_temp), make_reversed_pattern<typename decltype(__chunk1_temp)::as_simd>{});
				}(std::integral_constant<sizetype, _Indices_>{}), ...);
		}(std::make_integer_sequence<sizetype, _Simd_::__chunks_count()>{});

		return __result;
	}
}

__RAZE_VX_NAMESPACE_END
