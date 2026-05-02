#pragma once 

#include <src/raze/vx/hw/x86/cast/As.h>
#include <src/raze/vx/hw/x86/merge/Select.h>
#include <src/raze/vx/hw/x86/bitwise/BitOr.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Mask_or {
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
			return _Or<_ISA_, _Type_>()(__x, __y);
	}

	template <raw_mask_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_static_operator raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y, _Mask_ __mask) raze_const_operator noexcept {
		return _And<_ISA_, _Type_>()((*this)(__x, __y), __mask);
	}
};

__RAZE_VX_NAMESPACE_END
