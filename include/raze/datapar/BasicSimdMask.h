#pragma once 

#include <src/raze/datapar/SimdElementMaskOperations.h>
#include <src/raze/type_traits/TypeTraits.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class simd_mask: 
	public __simd_element_mask_operations<simd_mask<_ISA_, _Type_, _SimdWidth_>>
{
	static_assert(type_traits::__is_generation_supported_v<_ISA_>);
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);

	using __base = __simd_element_mask_operations<simd_mask<_ISA_, _Type_, _SimdWidth_>>;
public:
	static constexpr auto __isa = _ISA_;
	static constexpr auto __width = _SimdWidth_;

	using element_type	= _Type_;
	using mask_type		= type_traits::__deduce_simd_mask_type<_ISA_, element_type, _SimdWidth_>;

	static constexpr uint8 __used_bits = (_SimdWidth_ / 8) / sizeof(_Type_);

	simd_mask() noexcept;

	simd_mask(const mask_type __mask) noexcept;
	simd_mask(const simd_mask& __mask) noexcept;
	
	template <class _VectorMask_, std::enable_if_t<__is_valid_simd_v<_VectorMask_> || __is_intrin_type_v<_VectorMask_>, int> = 0>
	simd_mask(const _VectorMask_& __vector_mask) noexcept;

	constexpr raze_always_inline bool operator[](int32 __index) const noexcept;

	constexpr raze_always_inline simd_mask operator++(int) noexcept;
	constexpr raze_always_inline simd_mask& operator++() noexcept;

	constexpr raze_always_inline simd_mask operator--(int) noexcept;
	constexpr raze_always_inline simd_mask& operator--() noexcept;

	constexpr raze_always_inline simd_mask& operator&=(const simd_mask& __other) noexcept;
	constexpr raze_always_inline simd_mask& operator|=(const simd_mask& __other) noexcept;
	constexpr raze_always_inline simd_mask& operator^=(const simd_mask& __other) noexcept;

	constexpr raze_always_inline simd_mask& operator>>=(const uint8 __shift) const noexcept;
	constexpr raze_always_inline simd_mask& operator<<=(const uint8 __shift) const noexcept;

	template <uint8 _Shift_>
	constexpr raze_always_inline simd_mask& operator>>=(const std::integral_constant<uint8, _Shift_> __shift) const noexcept;
	
	template <uint8 _Shift_>
	constexpr raze_always_inline simd_mask& operator<<=(const std::integral_constant<uint8, _Shift_> __shift) const noexcept;

	static constexpr raze_always_inline uint8 bit_width() noexcept;
	static constexpr raze_always_inline arch::ISA isa() noexcept;
	constexpr raze_always_inline mask_type unwrap() const noexcept;

	constexpr raze_always_inline void clear_right_most_set_bit() noexcept;
	constexpr raze_always_inline void clear_left_most_set_bit() noexcept;
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END

#include <src/raze/datapar/BasicSimdMask.inl>