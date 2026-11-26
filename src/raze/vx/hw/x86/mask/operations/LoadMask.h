#pragma once 

#include <src/raze/vx/hw/x86/memory/Load.h>
#include <src/raze/vx/hw/x86/arithmetic/Negate.h>
#include <src/raze/vx/hw/x86/mask/operations/ToMask.h>
#include <src/raze/vx/hw/x86/mask/operations/ToVector.h>
#include <src/raze/vx/hw/x86/access/Insert.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, u32 N, raw_mask_type M, arithmetic_type T, class Policy = unaligned_policy>
raze_always_inline M load_mask_(const bool* mem, Policy policy = Policy{}) noexcept {
	if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
		return *mem;
	}
	else if constexpr (intrin_type<M>) {
		if constexpr (sizeof(T) == 1) {
			using IntVec = traits::deduce_simd_vector_type<i8, sizeof(M) * 8>;
			return as<M>(negate_<ISA, i8>(load_<ISA, IntVec>(mem, policy)));
		}
		else {
			using BitMask = std::conditional_t<(N <= 8), u8,
				std::conditional_t<(N <= 16), u16,
				std::conditional_t<(N <= 32), u32, u64>>>;
			BitMask mask = 0;

			for (u32 i = 0; i < N; ++i)
				if (mem[i])
					mask |= static_cast<BitMask>(BitMask(1) << i);

			return to_vector_<ISA, M, T>(mask);
		}
	}
	else if constexpr ((N == 16 && has_sse2<ISA>) || (N == 32 && has_avx2<ISA>) || (N == 64 && has_avx512bw<ISA>)) {
		using Vec = traits::deduce_simd_vector_type<i8, N * 8>;
		return static_cast<M>(to_mask_<ISA, i8>(negate_<ISA, i8>(load_<ISA, Vec>(mem, policy))));
	}
	else {
		M mask = 0;

		for (u32 i = 0; i < N; ++i)
			if (mem[i])
				mask |= static_cast<M>(M(1) << i);

		return mask;
	}
}

__RAZE_VX_NAMESPACE_END
