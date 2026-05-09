#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/Mul.h>
#include <src/raze/vx/hw/x86/arithmetic/Sub.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Fms {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Tp_ __z) const noexcept {
		if constexpr (sizeof(_Tp_) == 16 && __has_fma3_support<_ISA_>) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm_fmsub_pd(__as<__m128d>(__x), __as<__m128d>(__y), __as<__m128d>(__z)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm_fmsub_ps(__as<__m128>(__x), __as<__m128>(__y), __as<__m128>(__z)));
		}
		else if constexpr (sizeof(_Tp_) == 32 && __has_fma3_support<_ISA_>) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm256_fmsub_pd(__as<__m256d>(__x), __as<__m256d>(__y), __as<__m256d>(__z)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm256_fmsub_ps(__as<__m256>(__x), __as<__m256>(__y), __as<__m256>(__z)));
		}
		else if constexpr (sizeof(_Tp_) == 64) {
			if constexpr (__is_pd_v<_Type_>) return __as<_Tp_>(_mm512_fmsub_pd(__as<__m512d>(__x), __as<__m512d>(__y), __as<__m512d>(__z)));
			else if constexpr (__is_ps_v<_Type_>) return __as<_Tp_>(_mm512_fmsub_ps(__as<__m512>(__x), __as<__m512>(__y), __as<__m512>(__z)));
		}

		return _Sub<_ISA_, _Type_>()(_Mul<_ISA_, _Type_>()(__x, __y), __z);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Tp_ __z, _Mask_ __mask) const noexcept {
		return _Select<_ISA_, _Type_>()((*this)(__x, __y, __z), __mask);
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Tp_ __z, _Mask_ __mask, _Tp_ __src) const noexcept {
		return _Select<_ISA_, _Type_>()((*this)(__x, __y, __z), __src, __mask);
	}
};

__RAZE_VX_NAMESPACE_END
