#pragma once 

#include <src/raze/datapar/bitwise/ToMask.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA				_ISA_,
	std::unsigned_integral	_MaskType_>
raze_nodiscard raze_static_operator raze_always_inline
	__mmask_for_elements_t<sizeof(_MaskType_) * 8> __to_gpr(_MaskType_ __mask) noexcept
{
	if constexpr (sizeof(_MaskType_) == 1 && __has_avx512dq_support_v<_ISA_>)
		return _cvtu32_mask8(__mask);

	else if constexpr (sizeof(_MaskType_) == 2 && __has_avx512f_support_v<_ISA_>)
		return _cvtu32_mask16(__mask);

	else if constexpr (sizeof(_MaskType_) == 4 && __has_avx512bw_support_v<_ISA_>)
		return _cvtu32_mask32(__mask);

	else if constexpr (sizeof(_MaskType_) == 8 && __has_avx512bw_support_v<_ISA_>)
		return _cvtu64_mask64(__mask);

	else
		return __mask;
}

__RAZE_DATAPAR_NAMESPACE_END
