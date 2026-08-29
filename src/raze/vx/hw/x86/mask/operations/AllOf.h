#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/operations/ToBitmask.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M>
raze_always_inline bool none_of_(M __x) noexcept;

template <arch::ISA ISA, u64 N, arithmetic_type T, raw_mask_type M>
raze_always_inline bool all_of_(M x) noexcept {
	if constexpr (intrin_type<M>) {
		constexpr auto size = (sizeof(M)) == 64 ? 0xFFFFFFFFFFFFFFFFULL : u64((u64(1) << (sizeof(M))) - 1);
		return to_bitmask_<ISA, i8>(x) == size;
	}
	else if constexpr (std::is_integral_v<M> && !std::is_same_v<M, bool>) {
		raze_maybe_unused_attribute constexpr auto max_for_bits = ((sizeof(M) * 8) == N)
			? math::max_limit<M>() : M(((M(1) << N) - 1));

		if constexpr (N < 8 && has_avx512dq<ISA>) return _ktestc_mask8_u8(x, _cvtu32_mask8(max_for_bits));
		else if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>) return _kortestc_mask8_u8(x, x);
		else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>) return _kortestc_mask16_u8(x, x);
		else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>) return _kortestc_mask32_u8(x, x);
		else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>) return _kortestc_mask64_u8(x, x);
		else return (x == max_for_bits);
	}
	else return x;
}

template <arch::ISA ISA, u64 N, arithmetic_type T, raw_mask_type M, raw_mask_type Mask>
raze_always_inline bool all_of_(M x, Mask mask) noexcept
	requires((intrin_type<M> && intrin_type<Mask>) || (std::unsigned_integral<M> && std::unsigned_integral<Mask>
		&& !std::is_same_v<M, bool> && !std::is_same_v<M, bool>) || (std::is_same_v<M, bool> && std::is_same_v<M, Mask>))
{
    if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
        return !mask || x;
    }
    else if constexpr (std::is_integral_v<M> && !std::is_same_v<M, bool>) {
        if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>)
            return _ktestc_mask8_u8(x, mask);
        else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>)
            return _ktestc_mask16_u8(x, mask);
        else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>)
            return _ktestc_mask32_u8(x, mask);
        else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>)
            return _ktestc_mask64_u8(x, mask);
        else
            return (x & mask) == mask;
    }
	
	return none_of_<ISA, T>(mask_andnot_<ISA, T>(x, mask));
}

__RAZE_VX_NAMESPACE_END

#include <src/raze/vx/hw/x86/mask/operations/NoneOf.h>
