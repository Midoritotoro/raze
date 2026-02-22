#pragma once 

#include <src/raze/datapar/SimdIndexMaskOperations.h>
#include <src/raze/datapar/MaskTypeSelector.h>

#include <src/raze/datapar/bitwise/IndexMaskDivisor.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class simd_index_mask: public __simd_index_mask_operations<simd_index_mask<_ISA_, _Type_, _SimdWidth_>> {
public:
	static constexpr auto __isa = _ISA_;
	static constexpr auto __width = _SimdWidth_;

	static constexpr auto __is_k_register = __has_avx512f_support_v<__isa>;

	using element_type	= _Type_;

	static constexpr uint8 __divisor = __simd_index_mask_divisor_v<__isa, __width, element_type>;
	static constexpr uint8 __used_bits = (__width / 8) / sizeof(element_type) * __divisor;

	using mask_type = __mmask_for_size_t<((__used_bits <= 8) ? 1 : (__used_bits / 8))>;

	simd_index_mask() noexcept;
	simd_index_mask(const mask_type __mask) noexcept;
	
	template <class _VectorMask_, std::enable_if_t<__is_valid_simd_v<_VectorMask_> || __is_intrin_type_v<_VectorMask_>, int> = 0>
	simd_index_mask(const _VectorMask_& __vector_mask) noexcept;

	constexpr raze_always_inline simd_index_mask& operator&=(const simd_index_mask& __other) noexcept;
	constexpr raze_always_inline simd_index_mask& operator|=(const simd_index_mask& __other) noexcept;
	constexpr raze_always_inline simd_index_mask& operator^=(const simd_index_mask& __other) noexcept;

	constexpr raze_always_inline simd_index_mask& operator>>=(const uint8 __shift) noexcept;
	constexpr raze_always_inline simd_index_mask& operator<<=(const uint8 __shift) noexcept;

	template <sizetype _Shift_>
	constexpr raze_always_inline simd_index_mask& operator>>=(const std::integral_constant<uint8, _Shift_> __shift) noexcept;

	template <sizetype _Shift_>
	constexpr raze_always_inline simd_index_mask& operator<<=(const std::integral_constant<uint8, _Shift_> __shift) noexcept;

	constexpr raze_always_inline mask_type unwrap() const noexcept;
	static constexpr raze_always_inline arch::ISA isa() noexcept;
	static constexpr raze_always_inline uint8 bit_width() noexcept;
	static constexpr raze_always_inline uint8 divisor() noexcept;
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/SimdIndexMask.inl>