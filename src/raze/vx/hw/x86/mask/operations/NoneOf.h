#pragma once 

#include <src/raze/vx/hw/x86/compare/Equal.h>
#include <src/raze/vx/hw/x86/mask/operations/ToBitmask.h>
#include <src/raze/vx/hw/x86/construct/AllOnes.h>
#include <src/raze/vx/hw/x86/mask/operations/MaskAnd.h>

__RAZE_VX_NAMESPACE_BEGIN

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M>
raze_always_inline bool none_of_(M x) noexcept {
	if constexpr (intrin_type<M>) {
#if defined(raze_cpp_clang) || defined(raze_cpp_gnu)
		if constexpr (sizeof(M) == 16 && has_sse41<ISA>) return _mm_testz_si128(as<__m128i>(x), as<__m128i>(x));
		else if constexpr (sizeof(M) == 32) return _mm256_testz_si256(as<__m256i>(x), as<__m256i>(x));
		else return to_bitmask_<ISA, T>(x) == 0;
#elif defined(raze_cpp_msvc)
        return to_bitmask_<ISA, T>(x) == 0;
#endif
	}
	else if constexpr (std::is_integral_v<M> && !std::is_same_v<M, bool>) {
		if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>) return _kortestz_mask8_u8(x, x);
		else if constexpr (sizeof(M) == 2 && has_avx512f<ISA>) return _kortestz_mask16_u8(x, x);
		else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>) return _kortestz_mask32_u8(x, x);
		else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>) return _kortestz_mask64_u8(x, x);
		else return (x == 0);
	}
	else {
		return !x;
	}
}

template <arch::ISA ISA, arithmetic_type T, raw_mask_type M, raw_mask_type ControlMask>
raze_always_inline bool none_of_(M x, ControlMask mask) noexcept 
	requires((intrin_type<M> && intrin_type<ControlMask>) || (std::unsigned_integral<M> && std::unsigned_integral<ControlMask>))
{
    if constexpr (std::is_same_v<std::remove_cvref_t<M>, bool>) {
        return !mask || !x;
    }
    else if constexpr (intrin_type<M> && intrin_type<ControlMask>) {
        if constexpr (sizeof(M) == 16 && has_sse41<ISA>)
            return _mm_testz_si128(as<__m128i>(x), as<__m128i>(mask));
        else if constexpr (sizeof(M) == 32 && has_avx2<ISA>)
            return _mm256_testz_si256(as<__m256i>(x), as<__m256i>(mask));
        else
            return none_of_<ISA, T>(mask_and_<ISA, T>(x, mask));
    }
    else if constexpr (std::is_integral_v<M> && !std::is_same_v<M, bool>) {
        if constexpr (sizeof(M) == 1 && has_avx512dq<ISA>)
            return _ktestz_mask8_u8(x, mask);
        else if constexpr (sizeof(M) == 2 && has_avx512dq<ISA>)
            return _ktestz_mask16_u8(x, mask);
        else if constexpr (sizeof(M) == 4 && has_avx512bw<ISA>)
            return _ktestz_mask32_u8(x, mask);
        else if constexpr (sizeof(M) == 8 && has_avx512bw<ISA>)
            return _ktestz_mask64_u8(x, mask);
        else
            return (x & mask) == 0;
    }
}

__RAZE_VX_NAMESPACE_END
