#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/access/Extract.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>
#include <src/raze/vx/hw/x86/access/Extract.h>


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _I_>
consteval auto __broadcast_pshufd_index(std::integral_constant<sizetype, _I_> __i) noexcept {
	return (__i & 0x03) | (__i << 2) | (__i << 4) | (__i << 6);
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Tp_, sizetype _I_>
raze_always_inline auto __broadcast_element_native(_Tp_ __x, std::integral_constant<sizetype, _I_> __i) noexcept
	requires(__i >= 0 && __i < (sizeof(_Tp_) / sizeof(_Type_)))
{
	if constexpr (sizeof(_Tp_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_Tp_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), __i));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_Tp_>(_mm_shuffle_epi32(__as<__m128i>(__x), __broadcast_pshufd_index(__i)));
		}
		else if constexpr (sizeof(_Type_) == 1) {
			__m128i __combined;
			
			if constexpr (__i >= 8) __combined = _mm_unpacklo_epi8(__as<__m128i>(__x), __as<__m128i>(__x));
			else __combined = _mm_unpackhi_epi8(__as<__m128i>(__x), __as<__m128i>(__x));

			return __generic_shuffle_native<_ISA_, i16, _Tp_>(__combined, make_broadcast_pattern<
				simd<i16, runtime_abi<arch::ISA::SSE2, 8>>, __i % 8>{});
		}
	}
	else if constexpr (sizeof(_Tp_) == 32) {
		if constexpr (__i == 0) {
			if constexpr (sizeof(_Type_) == 8) {
				return __as<_Tp_>(_mm256_broadcastq_epi64(__as<__m128i>(__x)));
			}
			else if constexpr (sizeof(_Type_) == 4) {
				return __as<_Tp_>(_mm256_broadcastd_epi32(__as<__m128i>(__x)));
			}
			else if constexpr (sizeof(_Type_) == 2 && __has_avx2_support_v<_ISA_>) {
				return __as<_Tp_>(_mm256_broadcastw_epi16(__as<__m128i>(__x)));
			}
			else if constexpr (sizeof(_Type_) == 1 && __has_avx2_support_v<_ISA_>) {
				return __as<_Tp_>(_mm256_broadcastb_epi8(__as<__m128i>(__x)));
			}
		}
		else if constexpr (sizeof(_Type_) == 8) {
			return _mm256_permute4x64_epi64(__as<__m256i>(__x), __broadcast_pshufd_index(__i));
		}
		else {
			return _Broadcast<_ISA_, _Tp_>()(_Extract<_ISA_, _Type_>()(__x, __i));
		}
	}
	else if constexpr (sizeof(_Tp_) == 64) {
		if constexpr (__i == 0) {
			if constexpr (sizeof(_Type_) == 8) {
				return __as<_Tp_>(_mm512_broadcastq_epi64(__as<__m128i>(__x)));
			}
			else if constexpr (sizeof(_Type_) == 4) {
				return __as<_Tp_>(_mm512_broadcastd_epi32(__as<__m128i>(__x)));
			}
			else if constexpr (sizeof(_Type_) == 2 && __has_avx512bw_support_v<_ISA_>) {
				return __as<_Tp_>(_mm512_broadcastw_epi16(__as<__m128i>(__x)));
			}
			else if constexpr (sizeof(_Type_) == 1 && __has_avx512bw_support_v<_ISA_>) {
				return __as<_Tp_>(_mm512_broadcastb_epi8(__as<__m128i>(__x)));
			}
		}
		else {
			return _Broadcast<_ISA_, _Tp_>()(_Extract<_ISA_, _Type_>()(__x, __i));
		}
	}
}

template <class _Pattern_>
raze_always_inline pattern_vector_t<_Pattern_> __broadcast_element(const pattern_vector_t<_Pattern_>& __x, _Pattern_ __p) noexcept {
	using _Tp_ = pattern_vector_t<_Pattern_>;

	if constexpr (_Tp_::is_native()) {
		using _Ret = decltype(__broadcast_element_native<abi_t<_Tp_>::isa, typename _Tp_::value_type>(__storage_unwrap(__x.template __get<0>()), __p.at<0>()));

		if constexpr (!std::is_void_v<_Ret>) {
			auto __r = __x;

			auto& __storage = __r.template __get<0>();
			__storage = __broadcast_element_native<abi_t<_Tp_>::isa, typename _Tp_::value_type>(__storage_unwrap(__storage), __p.at<0>());

			return __r;
		}
		else
			return __generic_shuffle(__x, __p);
	}

	return __x[__p.at<0>()];
}

__RAZE_VX_NAMESPACE_END
