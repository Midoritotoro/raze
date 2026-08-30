#pragma once 

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAndnot.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskXor.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/math/SignBit.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, sizetype N, bool Unsafe, raw_mask_type M>
raze_always_inline M clear_last_(M x) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return 0;
	}
	else if constexpr (std::is_integral_v<M>) {
		if constexpr (N >= 32) {
			return mask_xor_<ISA, T>(x, (math::sign_bit<M>() >>
				math::clz_n_bits<ISA, N, Unsafe>(x)));
		}
		else {
			const auto pos = (N - 1) - math::clz_n_bits<ISA, N, Unsafe>(x);
			return mask_andnot_<ISA, T>(M(M(1) << pos), x);
		}
	}
	else if constexpr (intrin_type<M>) {
		return to_vector_<ISA, M, T>(clear_last_<ISA, T, N, Unsafe>(to_mask_<ISA, T>()(x)));
	}
}

__RAZE_VX_NAMESPACE_END
