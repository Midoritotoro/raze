#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/operations/ToBitmask.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, u64 _Size_, arithmetic_type _Type_>
struct _All_of {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline bool operator()(_Tp_ __x) const noexcept {
		if constexpr (intrin_type<_Tp_>) {
			if constexpr (sizeof(_Tp_) == 16) {
				if constexpr (__has_sse41_support_v<_ISA_>) return _mm_testc_si128(__as<__m128i>(__x), _All_ones<_ISA_, __m128i>()());
				else {
					const auto __all_ones = _Equal<arch::ISA::SSE2, i32>()(__x, __x);
					const auto __compared = _Equal<arch::ISA::SSE2, i32>()(__x, __all_ones);

					return _To_bitmask<arch::ISA::SSE2, i32>()(__compared) == 0xF;
				}
			}
			else if constexpr (sizeof(_Tp_) == 32) return _mm256_testc_si256(__as<__m256i>(__x), _All_ones<_ISA_, __m256i>()());
			else if constexpr (sizeof(_Tp_) == 64) return _Equal<_ISA_, i32>()(__x, _All_ones<_ISA_, _Tp_>()()) == 0xFFFF;
		}
		else if constexpr (std::is_integral_v<_Tp_> && !std::is_same_v<_Tp_, bool>) {
			raze_maybe_unused_attribute constexpr auto __max_for_bits = ((sizeof(_Tp_) * 8) == _Size_)
				? math::__maximum_integral_limit<_Tp_>() : _Tp_(((_Tp_(1) << _Size_) - 1));

			if constexpr (_Size_ < 8 && __has_avx512dq_support_v<_ISA_>) return _ktestc_mask8_u8(__x, _cvtu32_mask8(__max_for_bits));
			else if constexpr (sizeof(_Tp_) == 1 && __has_avx512dq_support_v<_ISA_>) return _kortestc_mask8_u8(__x, __x);
			else if constexpr (sizeof(_Tp_) == 2 && __has_avx512f_support_v<_ISA_>) return _kortestc_mask16_u8(__x, __x);
			else if constexpr (sizeof(_Tp_) == 4 && __has_avx512bw_support_v<_ISA_>) return _kortestc_mask32_u8(__x, __x);
			else if constexpr (sizeof(_Tp_) == 8 && __has_avx512bw_support_v<_ISA_>) return _kortestc_mask64_u8(__x, __x);
			else return (__x == __max_for_bits);
		}
		else return __x;
	}
};

__RAZE_VX_NAMESPACE_END
