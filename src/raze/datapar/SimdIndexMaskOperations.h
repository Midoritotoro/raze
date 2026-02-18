#pragma once 

#include <src/raze/datapar/SimdMaskCommonOperations.h>
#include <raze/math/BitMath.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Derived_>
struct __simd_index_mask_operations:
	public __simd_mask_common_operations<_Derived_>
{
    using __self = std::remove_cvref_t<_Derived_>;
	using __base = __simd_mask_common_operations<_Derived_>;

	static constexpr raze_always_inline uint8 __divisor() noexcept;

	constexpr raze_always_inline uint8 count_set() const noexcept;

	constexpr raze_always_inline uint8 count_trailing_zero_bits() const noexcept;
	constexpr raze_always_inline uint8 count_leading_zero_bits() const noexcept;

	constexpr raze_always_inline uint8 count_trailing_one_bits() const noexcept;
	constexpr raze_always_inline uint8 count_leading_one_bits() const noexcept;
};

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/SimdIndexMaskOperations.inl>
