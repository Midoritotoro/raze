#pragma once 

#include <src/raze/vx/hw/x86/memory/Store.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/math/BitTest.h>


__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, arithmetic_type T, raw_mask_type M, class Policy = unaligned_policy>
raze_always_inline void store_mask_(bool* mem, M mask, Policy policy = Policy{}) noexcept {
	using Signed = typename IntegerForSizeof<T>::Signed;

	if constexpr (intrin_type<M>)
		store_(mem, negate_<ISA, Signed>(mask), policy);
	else if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>)
		*mem = mask;
	else if constexpr (N >= 16)
		store_(mem, negate_<ISA, byte>(to_vector_<ISA, 
			traits::deduce_simd_vector_type<Signed, N * 8>, byte>(mask)), policy);
	else {
		for (auto i = 0; i < N; ++i)
			*mem++ = static_cast<bool>(math::bit_test(mask, i));
	}
}

__RAZE_VX_NAMESPACE_END
