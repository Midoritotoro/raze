#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/operations/ToBitmask.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _None_of {
	template <raw_mask_type _Tp_>
	raze_nodiscard raze_always_inline bool operator()(_Tp_ __x) const noexcept {
		if constexpr (intrin_type<_Tp_>) {
			//if constexpr (sizeof(_Tp_) == 16 && __has_sse41_support_v<_ISA_>) {
			//	return _mm_testz_si128(__as<__m128i>(__x), __as<__m128i>(__x));
			//}
			//else if constexpr (sizeof(_Tp_) == 32) {
			//	return _mm256_testz_si256(__as<__m256i>(__x), __as<__m256i>(__x));
			//}
			return _To_bitmask<_ISA_, _Type_>()(__x) == 0;
		}
		else if constexpr (std::is_integral_v<_Tp_> && !std::is_same_v<_Tp_, bool>) {
			if constexpr (sizeof(_Tp_) == 1 && __has_avx512dq_support_v<_ISA_>)
				return _kortestz_mask8_u8(__x, __x);

			else if constexpr (sizeof(_Tp_) == 2 && __has_avx512f_support_v<_ISA_>)
				return _kortestz_mask16_u8(__x, __x);

			else if constexpr (sizeof(_Tp_) == 4 && __has_avx512bw_support_v<_ISA_>)
				return _kortestz_mask32_u8(__x, __x);

			else if constexpr (sizeof(_Tp_) == 8 && __has_avx512bw_support_v<_ISA_>)
				return _kortestz_mask64_u8(__x, __x);

			else
				return (__x == 0);
		}
		else {
			return !__x;
		}
	}

	template <wrapped_type _Type_>
	raze_nodiscard raze_always_inline bool operator()(const _Type_& __x) const noexcept {
		return (*this)(__x.data());
	}
};

__RAZE_VX_NAMESPACE_END
