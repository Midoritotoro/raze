#pragma once 

#include <raze/vx/Abi.h>
#include <raze/vx/Mask.h>

__RAZE_VX_NAMESPACE_BEGIN

template <class _Simd_>
concept simd_type = __is_valid_simd_v<_Simd_>;

template <class _Type_>
concept simd_or_arithmetic_type = simd_type<_Type_> || arithmetic_type<_Type_>;

template <class _SimdMask_>
concept simd_mask_type = __is_simd_mask_v<_SimdMask_>;

template <class _SimdOrMask_>
concept simd_or_mask_type = simd_mask_type<_SimdOrMask_> || simd_type<_SimdOrMask_>;

template <class _AlignmentPolicy_>
concept alignment_policy_type = requires {
	{ std::remove_cvref_t<_AlignmentPolicy_>::__alignment } -> std::convertible_to<bool>;
};

struct aligned_mode {};
constexpr inline auto aligned = raze::options::flag(aligned_mode{});
struct aligned_option : raze::options::exact_option<aligned> {};

__RAZE_VX_NAMESPACE_END
