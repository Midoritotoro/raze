#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/operations/ToBitmask.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_, raw_mask_type _Tp_>
raze_nodiscard raze_always_inline bool __none_of(_Tp_ __x) noexcept;

template <arch::ISA _ISA_, u64 _Size_, arithmetic_type _Type_, raw_mask_type _Tp_>
raze_nodiscard raze_always_inline bool __all_of(_Tp_ __x) noexcept {
	if constexpr (intrin_type<_Tp_>) {
		constexpr auto __size = (sizeof(_Tp_) / sizeof(_Type_)) == 64 ? 0xFFFFFFFFFFFFFFFFULL : u64((u64(1) << (sizeof(_Tp_) / sizeof(_Type_))) - 1);
		return _To_bitmask<_ISA_, _Type_>()(__x) == __size;
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

template <arch::ISA _ISA_, u64 _Size_, arithmetic_type _Type_, raw_mask_type _Tp_, raw_mask_type _Mask_>
raze_nodiscard raze_always_inline bool __all_of(_Tp_ __x, _Mask_ __mask) noexcept
	requires((intrin_type<_Tp_> && intrin_type<_Mask_>) || (std::unsigned_integral<_Tp_> && std::unsigned_integral<_Mask_>
		&& !std::is_same_v<_Tp_, bool> && !std::is_same_v<_Tp_, bool>) || (std::is_same_v<_Tp_, bool> && std::is_same_v<_Tp_, _Mask_>))
{
	if constexpr (std::is_same_v<std::remove_cvref_t<_Tp_>, bool>) { return !__mask || __x; }
	else return __none_of<_ISA_, _Type_>(_Mask_andnot<_ISA_, _Type_>()(__x, __mask));
}

__RAZE_VX_NAMESPACE_END

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
