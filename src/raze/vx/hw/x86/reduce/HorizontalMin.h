#pragma once 

#include <src/raze/vx/hw/x86/arithmetic/VerticalMin.h>
#include <src/raze/vx/hw/x86/reduce/Fold.h>
#include <src/raze/vx/hw/x86/merge/Select.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V>
raze_always_inline T horizontal_min_(V x) noexcept {
	return mirror_fold_<ISA, T>(x, [] (auto x, auto y) raze_always_inline_lambda { 
		return vertical_min_<ISA, T>(x, y);
	});
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline T horizontal_min_(V x, M mask) noexcept {
	return mirror_fold_<ISA, T>(select_<ISA, T>(x, mask), [] (auto x, auto y) raze_always_inline_lambda { 
		return vertical_min_<ISA, T>(x, y);
	});
}

template <arch::ISA	ISA, arithmetic_type T, intrin_or_arithmetic_type V, raw_mask_type M>
raze_always_inline T horizontal_min_(V x, M mask, V src) noexcept {
	return mirror_fold_<ISA, T>(select_<ISA, T>(x, src, mask), [] (auto x, auto y) raze_always_inline_lambda { 
		return vertical_min_<ISA, T>(x, y);
	});
}

__RAZE_VX_NAMESPACE_END
