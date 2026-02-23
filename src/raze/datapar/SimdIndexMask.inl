#pragma once 

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
simd_index_mask<_ISA_, _Type_, _SimdWidth_>::simd_index_mask() noexcept
{}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
template <class _VectorMask_, std::enable_if_t<__is_valid_simd_v<_VectorMask_> || __is_intrin_type_v<_VectorMask_>, int>>
simd_index_mask<_ISA_, _Type_, _SimdWidth_>::simd_index_mask(const _VectorMask_& __vector_mask) noexcept {
	if constexpr (__is_valid_simd_v<_VectorMask_>)
		_mask = __vector_mask.to_index_mask().unwrap();
	else
		_mask = __simd_to_index_mask<_ISA_, _SimdWidth_, _Type_>(__vector_mask);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
simd_index_mask<_ISA_, _Type_, _SimdWidth_>::simd_index_mask(const mask_type __mask) noexcept :
	_mask(__mask)
{}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>::mask_type
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::unwrap() const noexcept 
{
	return _mask;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr arch::ISA simd_index_mask<_ISA_, _Type_, _SimdWidth_>::isa() noexcept {
	return _ISA_;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr uint8 simd_index_mask<_ISA_, _Type_, _SimdWidth_>::bit_width() noexcept {
	return __used_bits;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr uint8 simd_index_mask<_ISA_, _Type_, _SimdWidth_>::elements() noexcept {
	return __used_bits / __divisor;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr uint8 simd_index_mask<_ISA_, _Type_, _SimdWidth_>::divisor() noexcept {
	return __divisor;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>&
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator&=(const simd_index_mask& __other) noexcept
{
	return *this = (*this & __other);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>& 
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator|=(const simd_index_mask& __other) noexcept
{
	return *this = (*this | __other);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>& 
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator^=(const simd_index_mask& __other) noexcept
{
	return *this = (*this ^ __other);
}


template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>& 
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator>>=(const uint8 __shift) noexcept
{
	return *this = (*this >> __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>& 
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator<<=(const uint8 __shift) noexcept 
{
	return *this = (*this << __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
template <sizetype _Shift_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>& 
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator>>=(const std::integral_constant<uint8, _Shift_> __shift) noexcept 
{
	return *this = (*this >> __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
template <sizetype _Shift_>
constexpr simd_index_mask<_ISA_, _Type_, _SimdWidth_>& 
	simd_index_mask<_ISA_, _Type_, _SimdWidth_>::operator<<=(const std::integral_constant<uint8, _Shift_> __shift) noexcept
{
	return *this = (*this << __shift);
}

__RAZE_DATAPAR_NAMESPACE_END
