#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA		_ISA_,
	arithmetic_type	_Type_>
struct _Or {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		using _Unsigned = typename IntegerForSizeof<_Type_>::Unsigned;

		if constexpr (std::is_same_v<_Tp_, __m128d>) return _mm_or_pd(__x, __y);
		else if constexpr (std::is_same_v<_Tp_, __m128i>) return _mm_or_si128(__x, __y);
		else if constexpr (std::is_same_v<_Tp_, __m128>) return _mm_or_ps(__x, __y);
		else if constexpr (std::is_same_v<_Tp_, __m256d>) return _mm256_or_pd(__x, __y);
		else if constexpr (__has_avx2_support_v<_ISA_> && std::is_same_v<_Tp_, __m256i>) return _mm256_or_si256(__x, __y);
		else if constexpr (sizeof(_Tp_) == 32) return __as<_Tp_>(_mm256_or_ps(__as<__m256>(__x), __as<__m256>(__y)));
		else if constexpr (std::is_same_v<_Tp_, __m512d>) return _mm512_or_pd(__x, __y);
		else if constexpr (std::is_same_v<_Tp_, __m512i>) return _mm512_or_si512(__x, __y);
		else if constexpr (std::is_same_v<_Tp_, __m512>) return _mm512_or_ps(__x, __y);
		else return math::bit_cast<_Tp_>(_Unsigned(math::bit_cast<_Unsigned>(__x) | math::bit_cast<_Unsigned>(__y)));
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask, _Tp_ __src) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm_mask_or_epi64(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm_mask_or_epi32(__as<__m128i>(__src), __mask, __as<__m128i>(__x), __as<__m128i>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm256_mask_or_epi64(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm256_mask_or_epi32(__as<__m256i>(__src), __mask, __as<__m256i>(__x), __as<__m256i>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 64 && __has_avx512f_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm512_mask_or_epi64(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm512_mask_or_epi32(__as<__m512i>(__src), __mask, __as<__m512i>(__x), __as<__m512i>(__y)));
		}

		return _Select<_ISA_, _Type_>()((*this)(__x, __y), __src, __mask);
	}

	template <
		intrin_or_arithmetic_type	_Tp_,
		raw_mask_type				_Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm_maskz_or_epi64(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm_maskz_or_epi32(__mask, __as<__m128i>(__x), __as<__m128i>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_avx512vl_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm256_maskz_or_epi64(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm256_maskz_or_epi32(__mask, __as<__m256i>(__x), __as<__m256i>(__y)));
		}
		else if constexpr (sizeof(_Tp_) == 64 && __has_avx512f_support_v<_ISA_> && std::is_integral_v<_Mask_>) {
			if constexpr (sizeof(_Type_) == 8) return __as<_Tp_>(_mm512_maskz_or_epi64(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
			else if constexpr (sizeof(_Type_) == 4) return __as<_Tp_>(_mm512_maskz_or_epi32(__mask, __as<__m512i>(__x), __as<__m512i>(__y)));
		}

		return _Selectz<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}

	template <raw_mask_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) raze_const_operator noexcept {
		if constexpr (sizeof(_Tp_) == 1 && __has_avx512dq_support_v<_ISA_>)
			return _kor_mask8(__x, __y);

		else if constexpr (sizeof(_Tp_) == 2 && __has_avx512f_support_v<_ISA_>)
			return _kor_mask16(__x, __y);

		else if constexpr (sizeof(_Tp_) == 4 && __has_avx512bw_support_v<_ISA_>)
			return _kor_mask32(__x, __y);

		else if constexpr (sizeof(_Tp_) == 8 && __has_avx512bw_support_v<_ISA_>)
			return _kor_mask64(__x, __y);

		else
			return (*this)(__x, __y);
	}

	template <raw_mask_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) raze_const_operator noexcept {
		return _And<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}
};

__RAZE_VX_NAMESPACE_END
