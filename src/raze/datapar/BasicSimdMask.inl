#pragma once 

__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
simd_mask<_ISA_, _Type_, _SimdWidth_>::simd_mask() noexcept
{}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
simd_mask<_ISA_, _Type_, _SimdWidth_>::simd_mask(const mask_type __mask) noexcept :
	_mask(__mask)
{}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
simd_mask<_ISA_, _Type_, _SimdWidth_>::simd_mask(const simd_mask& __mask) noexcept :
	_mask(__mask._mask)
{}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>::mask_type
	simd_mask<_ISA_, _Type_, _SimdWidth_>::unwrap() const noexcept
{
	return _mask;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr bool simd_mask<_ISA_, _Type_, _SimdWidth_>::operator[](int32 __index) const noexcept {
	return (_mask >> __index) & 1;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_> simd_mask<_ISA_, _Type_, _SimdWidth_>::operator++(int) noexcept {
	simd_mask __self = *this;
	++_mask;
	return __self;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>& simd_mask<_ISA_, _Type_, _SimdWidth_>::operator++() noexcept {
	++_mask;
	return *this;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_> simd_mask<_ISA_, _Type_, _SimdWidth_>::operator--(int) noexcept {
	simd_mask __self = *this;
	--_mask;
	return __self;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>& simd_mask<_ISA_, _Type_, _SimdWidth_>::operator--() noexcept {
	--_mask;
	return *this;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
	simd_mask<_ISA_, _Type_, _SimdWidth_>::operator&=(const simd_mask& __other) noexcept
{
	return *this = (*this & __other);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
	simd_mask<_ISA_, _Type_, _SimdWidth_>::operator|=(const simd_mask& __other) noexcept
{
	return *this = (*this | __other);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
	simd_mask<_ISA_, _Type_, _SimdWidth_>::operator^=(const simd_mask& __other) noexcept
{
	return *this = (*this ^ __other);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
	simd_mask<_ISA_, _Type_, _SimdWidth_>::operator>>=(const uint8 __shift) const noexcept
{
	return *this = (*this >> __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
	simd_mask<_ISA_, _Type_, _SimdWidth_>::operator<<=(const uint8 __shift) const noexcept
{
	return *this = (*this << __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
template <uint8 _Shift_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
simd_mask<_ISA_, _Type_, _SimdWidth_>::operator>>=(const std::integral_constant<uint8, _Shift_> __shift) const noexcept
{
	return *this = (*this >> __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
template <uint8 _Shift_>
constexpr simd_mask<_ISA_, _Type_, _SimdWidth_>&
simd_mask<_ISA_, _Type_, _SimdWidth_>::operator<<=(const std::integral_constant<uint8, _Shift_> __shift) const noexcept
{
	return *this = (*this << __shift);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr uint8 simd_mask<_ISA_, _Type_, _SimdWidth_>::bit_width() noexcept {
	return __used_bits;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr arch::ISA simd_mask<_ISA_, _Type_, _SimdWidth_>::isa() noexcept {
	return _ISA_;
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr void simd_mask<_ISA_, _Type_, _SimdWidth_>::clear_right_most_set_bit() noexcept {
	_mask &= (_mask - 1);
}

template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
constexpr void simd_mask<_ISA_, _Type_, _SimdWidth_>::clear_left_most_set_bit() noexcept {
	_mask &= (_mask - (mask_type(1) << (bit_width() - 1 - __base::count_leading_zero_bits())));
}

__RAZE_DATAPAR_NAMESPACE_END
