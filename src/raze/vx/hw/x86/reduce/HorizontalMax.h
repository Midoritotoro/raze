#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/VerticalMax.h>
#include <src/raze/vx/hw/x86/reduce/Fold.h>


__RAZE_VX_NAMESPACE_BEGIN 

template <arch::ISA _ISA_, arithmetic_type _Type_>
struct _Vmax_wrapper {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Tp_ operator()(_Tp_ __x, _Tp_ __y) const noexcept {
		return _Vertical_max<_ISA_, _Type_>()(__x, __y);
	}
};

template <arch::ISA	_ISA_, arithmetic_type _Type_>
struct _Horizontal_max {
	template <intrin_or_arithmetic_type _Tp_>
	raze_nodiscard raze_always_inline _Type_ operator()(_Tp_ __x) const noexcept {
		return _Fold<_ISA_, _Type_>()(__x, _Vmax_wrapper<_ISA_, _Type_>{});
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Type_ operator()(_Tp_ __x, _Mask_ __mask) const noexcept {
		return _Fold<_ISA_, _Type_>()(_Selectz<_ISA_, _Type_>()(__x, __mask), _Vmax_wrapper<_ISA_, _Type_>{});
	}

	template <intrin_or_arithmetic_type _Tp_, raw_mask_type _Mask_>
	raze_nodiscard raze_always_inline _Type_ operator()(_Tp_ __x, _Mask_ __mask, _Tp_ __src) const noexcept {
		return _Fold<_ISA_, _Type_>()(_Select<_ISA_, _Type_>()(__x, __src, __mask), _Vmax_wrapper<_ISA_, _Type_>{});
	}
};

__RAZE_VX_NAMESPACE_END
