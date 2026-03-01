#pragma once 

#include <src/raze/datapar/SimdElementMaskOperations.h>
#include <src/raze/type_traits/TypeTraits.h>
#include <src/raze/datapar/Compare.h>
#include <src/raze/datapar/MaskTypeSelector.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Simd_>
constexpr inline auto __is_native_compare_returns_number_v = std::is_integral_v<
	type_traits::invoke_result_type<_Simd_equal<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>,
	typename _Simd_::vector_type, typename _Simd_::vector_type>>;

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
	static constexpr auto __number_mask = __is_native_compare_returns_number_v<simd<_ISA_, _Type_, _SimdWidth_>>;
public:
	static constexpr auto __isa = _ISA_;
	static constexpr auto __width = _SimdWidth_;
	static constexpr auto __is_k_register = __has_avx512f_support_v<__isa>;
	static constexpr auto __used_bits = __number_mask ? (_SimdWidth_ / 8) / sizeof(_Type_) : _SimdWidth_;

	using element_type	= _Type_;
	using mask_type = std::conditional_t<__number_mask,
		__mmask_for_size_t<((__used_bits <= 8) ? 1 : (__used_bits / 8))>,
		typename simd<_ISA_, typename IntegerForSizeof<_Type_>::Unsigned, __used_bits>::vector_type>;

	simd_mask() noexcept;

	simd_mask(const mask_type __mask) noexcept;
	simd_mask(const simd_mask& __mask) noexcept;
	
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