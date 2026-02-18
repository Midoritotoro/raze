__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Derived_>
constexpr arch::ISA __simd_mask_common_operations<_Derived_>::__isa() noexcept {
	return __self::isa();
}

template <class _Derived_>
constexpr auto __simd_mask_common_operations<_Derived_>::__unwrap() const noexcept {
	return static_cast<const __self&>(*this).unwrap();
}

template <class _Derived_>
constexpr uint8 __simd_mask_common_operations<_Derived_>::__bit_width() noexcept {
	return __self::bit_width();
}

template <class _Derived_>
constexpr bool __simd_mask_common_operations<_Derived_>::all_of() const noexcept {
	const auto __bits		= __unwrap();
	using _MaskType = decltype(__bits);

	constexpr auto __max_for_bits = ((sizeof(_MaskType) * 8) == __bit_width())
		? math::__maximum_integral_limit<_MaskType>()
		: _MaskType(((_MaskType(1) << __bit_width()) - 1));

	if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _ktestc_mask16_u8(__bits, __max_for_bits);

	else if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _ktestc_mask8_u8(__bits, __max_for_bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _ktestc_mask32_u8(__bits, __max_for_bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _ktestc_mask64_u8(__bits, __max_for_bits);

	else
		return (__bits == __max_for_bits);
}

template <class _Derived_>
constexpr bool __simd_mask_common_operations<_Derived_>::any_of() const noexcept {
	return !none_of();
}

template <class _Derived_>
constexpr bool __simd_mask_common_operations<_Derived_>::none_of() const noexcept {
	const auto __bits = __unwrap();
	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _kortestz_mask16_u8(__bits, __bits);

	else if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _kortestz_mask8_u8(__bits, __bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kortestz_mask32_u8(__bits, __bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _kortestz_mask64_u8(__bits, __bits);

	else
		return (__bits == 0);
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>::operator&(const __self& __other) const noexcept {
	const auto __bits		= __unwrap();
	const auto __other_bits = __other.__unwrap();

	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _kand_mask8(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _kand_mask16(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kand_mask32(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _kand_mask64(__bits, __other_bits);

	else
		return __bits & __other_bits;
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>::operator|(const __self& __other) const noexcept {
	const auto __bits		= __unwrap();
	const auto __other_bits = __other.__unwrap();

	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _kor_mask8(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _kor_mask16(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kor_mask32(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _kor_mask64(__bits, __other_bits);

	else
		return __bits | __other_bits;
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>::operator^(const __self& __other) const noexcept {
	const auto __bits		= __unwrap();
	const auto __other_bits = __other.__unwrap();

	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _kxor_mask8(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _kxor_mask16(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kxor_mask32(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _kxor_mask64(__bits, __other_bits);

	else
		return __bits ^ __other_bits;
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>::operator>>(const uint8 __shift) const noexcept {
	return __to_int() >> __shift;
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>::operator<<(const uint8 __shift) const noexcept {
	return __to_int() << __shift;
}

template <class _Derived_>
template <uint8 _Shift_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>
	::operator>>(const std::integral_constant<uint8, _Shift_> __shift) const noexcept
{
	const auto __bits = __unwrap();
	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _kshiftri_mask8(__bits, __shift);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _kshiftri_mask16(__bits, __shift);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kshiftri_mask32(__bits, __shift);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kshiftri_mask64(__bits, __shift);

	else 
		return __bits << __shift;

}

template <class _Derived_>
template <uint8 _Shift_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>
	::operator<<(const std::integral_constant<uint8, _Shift_> __shift) const noexcept
{
	const auto __bits = __unwrap();
	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _kshiftli_mask8(__bits, __shift);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _kshiftli_mask16(__bits, __shift);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _kshiftli_mask32(__bits, __shift);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _kshiftli_mask64(__bits, __shift);

	else
		return __bits >> __shift;
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::__self __simd_mask_common_operations<_Derived_>::operator~() const noexcept {
	const auto __bits = __unwrap();
	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _knot_mask8(__bits);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _knot_mask16(__bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _knot_mask32(__bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _knot_mask64(__bits);

	else
		return ~__bits;
}

template <class _Derived_>
constexpr auto __simd_mask_common_operations<_Derived_>::__to_kmask() const noexcept {
	const auto __bits = __unwrap();
	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _cvtmask8_u32(__bits);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _cvtmask16_u32(__bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _cvtmask32_u32(__bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _cvtmask64_u64(__bits);

	else
		return __bits;
}

template <class _Derived_>
constexpr auto __simd_mask_common_operations<_Derived_>::__to_int() const noexcept {
	const auto __bits = __unwrap();
	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _cvtu32_mask8(__bits);

	else if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _cvtu32_mask16(__bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _cvtu32_mask32(__bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _cvtu64_mask64(__bits);

	else
		return __bits;
}

template <class _Derived_>
constexpr __simd_mask_common_operations<_Derived_>::operator bool() const noexcept {
	return any_of();
}

template <class _Derived_>
constexpr bool __simd_mask_common_operations<_Derived_>::operator==(const __self& __other) const noexcept {
	const auto __bits = __unwrap();
	const auto __other_bits = __other.__unwrap();

	using _MaskType = decltype(__bits);

	if constexpr (sizeof(_MaskType) == 2 && __has_avx512f_support_v<__isa()>)
		return _ktestc_mask16_u8(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 1 && __has_avx512dq_support_v<__isa()>)
		return _ktestc_mask8_u8(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 4 && __has_avx512bw_support_v<__isa()>)
		return _ktestc_mask32_u8(__bits, __other_bits);

	else if constexpr (sizeof(_MaskType) == 8 && __has_avx512bw_support_v<__isa()>)
		return _ktestc_mask64_u8(__bits, __other_bits);

	else
		return (__bits == __other_bits);
}

template <class _Derived_>	
constexpr bool __simd_mask_common_operations<_Derived_>::operator!=(const __self& __other) const noexcept {	
	return !(*this == __other);
}

__RAZE_DATAPAR_NAMESPACE_END
