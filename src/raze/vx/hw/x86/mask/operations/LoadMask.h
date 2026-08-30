#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/access/Insert.h>
#include <src/raze/math/BitTestAndSet.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, raw_mask_type M, arithmetic_type T, class Policy = unaligned_policy>
raze_always_inline M load_mask_(const bool* mem, Policy policy = Policy{}) noexcept {
	using Signed = typename IntegerForSizeof<T>::Signed;

	if constexpr (intrin_type<M>)
		return negate_<ISA, Signed>(load_<ISA, M>(mem, policy));
	else if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>)
		return *mem;
	else if constexpr (N >= 16)
		return to_mask_<ISA, byte>(negate_<ISA, byte>(load_<ISA, 
			traits::deduce_simd_vector_type<Signed, N * 8>>()(mem, policy)));
	else {
		M mask = 0;

		for (auto i = 0; i < N; ++i)
			if (*mem++)
				math::bit_test_and_set(mask, i);

		return mask;
	}
}

__RAZE_VX_NAMESPACE_END
