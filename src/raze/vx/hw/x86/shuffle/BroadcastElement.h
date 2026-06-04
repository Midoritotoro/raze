#pragma once 

#include <src/raze/vx/hw/common/PatternsCheck.h>
#include <src/raze/vx/hw/x86/access/Extract.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/shuffle/GenericShuffle.h>


__RAZE_VX_NAMESPACE_BEGIN

template <sizetype _I_>
consteval auto __broadcast_pshufd_index(std::integral_constant<sizetype, _I_> __i) noexcept {
	return (__i & 0x03) | (__i << 2) | (__i << 4) | (__i << 6);
}

template <arch::ISA _ISA_, arithmetic_type _Type_, intrin_type _Tp_, sizetype _I_>
raze_always_inline _Tp_ __broadcast_element_native(_Tp_ __x, std::integral_constant<sizetype, _I_> __i) noexcept
	requires(__i >= 0 && __i < (sizeof(_Tp_) / sizeof(_Type_)))
{
	if constexpr (sizeof(_Tp_) == 16) {
		if constexpr (sizeof(_Type_) == 8) {
			return __as<_Tp_>(_mm_shuffle_pd(__as<__m128d>(__x), __as<__m128d>(__x), __i));
		}
		else if constexpr (sizeof(_Type_) == 4) {
			return __as<_Tp_>(_mm_shuffle_epi32(__broadcast_pshufd_index(__i)));
		}
		else if constexpr (sizeof(_Type_) == 2) {
			
		}
		else if constexpr (sizeof(_Type_) == 1) {
			
		}
	}
	else if constexpr (sizeof(_Tp_) == 32) {

	}
	else if constexpr (sizeof(_Tp_) == 64) {

	}
}

template <arch::ISA _ISA_, arithmetic_type _Type_, simd_type _Tp_, sizetype _I_>
raze_always_inline _Tp_ __broadcast_element(_Tp_ __x, std::integral_constant<sizetype, _I_> __i) noexcept {
	if constexpr (_Tp_::is_native()) {
		return __broadcast_element_native<_ISA_, _Type_>(__storage_unwrap(__x.template get<0>()), __i);
	}

	__x.__for_each_chunk([&] <class _Chunk> (_Chunk & __chunk) raze_always_inline_lambda {
		__chunk = __broadcast_element_native<_ISA_, _Type_>(__storage_unwrap(__chunk), __i);
	});

	return __x[__i];
}

__RAZE_VX_NAMESPACE_END
