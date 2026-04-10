#pragma once 

#include <src/raze/vx/bitwise/ToMask.h>


__RAZE_VX_NAMESPACE_BEGIN

template <
	arch::ISA				_ISA_,
	std::unsigned_integral	_MaskType_>
raze_nodiscard raze_static_operator raze_always_inline
	auto __to_k(_MaskType_ __mask) noexcept
{
	if constexpr (sizeof(_MaskType_) == 1 && __has_avx512dq_support_v<_ISA_>)
		return _cvtmask8_u32(__mask);

	else if constexpr (sizeof(_MaskType_) == 2 && __has_avx512f_support_v<_ISA_>)
		return _cvtmask16_u32(__mask);

	else if constexpr (sizeof(_MaskType_) == 4 && __has_avx512bw_support_v<_ISA_>)
		return _cvtmask32_u32(__mask);

	else if constexpr (sizeof(_MaskType_) == 8 && __has_avx512bw_support_v<_ISA_>)
		return _cvtmask64_u64(__mask);

	else
		return __mask;
}

__RAZE_VX_NAMESPACE_END
