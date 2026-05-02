#pragma once

#include <src/raze/vx/hw/x86/shuffle/SlideRight.h>
#include <src/raze/vx/hw/x86/mask/operations/BitAnd.h>
#include <raze/math/IntegralTypesConversions.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, intrin_type _Intrin_, arithmetic_type _Type_>
struct _Mask_slide_rigth {
	static constexpr auto __size = sizeof(_Intrin_) / sizeof(_Type_);

	template <raw_mask_type _Tp_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, i32 __shift) raze_const_operator noexcept {
		if constexpr (intrin_type<_Tp_>)
			return _Slide_right<_ISA_, _Type_>()(__x, __shift);
		else
			return (__shift >= __size) ? 0 : __x >> __shift;
	}

	template <raw_mask_type _Tp_, i32 _Shift_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x,
		std::integral_constant<i32, _Shift_> __shift) raze_const_operator noexcept 
	{
		if constexpr (__shift >= __size)
			return 0;

		if constexpr (sizeof(_Tp_) == 1 && __has_avx512dq_support_v<_ISA_>)
			return _kshiftri_mask8(__x, __shift);

		else if constexpr (sizeof(_Tp_) == 2 && __has_avx512f_support_v<_ISA_>)
			return _kshiftri_mask16(__x, __shift);

		else if constexpr (sizeof(_Tp_) == 4 && __has_avx512bw_support_v<_ISA_>)
			return _kshiftri_mask32(__x, __shift);

		else if constexpr (sizeof(_Tp_) == 8 && __has_avx512bw_support_v<_ISA_>)
			return _kshiftri_mask64(__x, __shift);

		else if constexpr (intrin_type<_Tp_>)
			return _Slide_right<_ISA_, _Type_>()(__x, __shift);

		else
			return __x >> __shift;
	}
};

__RAZE_VX_NAMESPACE_END
