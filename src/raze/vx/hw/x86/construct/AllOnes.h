#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/construct/Broadcast.h>
#include <src/raze/vx/hw/x86/construct/Zero.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA	ISA, intrin_or_arithmetic_type V>
raze_always_inline V all_ones_() noexcept {
	if constexpr (has_avx512f<ISA>) {
		return broadcast_<ISA, V>(-1);
	}
	else if constexpr (has_avx<ISA> && !has_avx2<ISA>) {
		using FV = traits::deduce_simd_vector_type<f32, raze_sizeof_in_bits(V)>;
		return as<V>(equal_<ISA, f32>(zero_<ISA, FV>(), zero_<ISA, FV>()));
	}
	else {
		using IV = traits::deduce_simd_vector_type<i32, raze_sizeof_in_bits(V)>;
		return as<V>(equal_<ISA, i32>(zero_<ISA, IV>(), zero_<ISA, IV>()));
	}
}

__RAZE_VX_NAMESPACE_END
