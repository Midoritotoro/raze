__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _Derived_>
constexpr uint8 __simd_element_mask_operations<_Derived_>::count_set() const noexcept {
	return math::__popcnt_n_bits<__base::__bit_width()>(__base::__to_int());
}

template <class _Derived_>
constexpr uint8 __simd_element_mask_operations<_Derived_>::count_trailing_zero_bits() const noexcept {
	if constexpr (__has_avx2_support_v<__base::__isa()>)
		return math::__tzcnt_ctz_unsafe(__base::__to_int());
	else
		return math::__bsf_ctz_unsafe(__base::__to_int());
}

template <class _Derived_>
constexpr uint8 __simd_element_mask_operations<_Derived_>::count_leading_zero_bits() const noexcept {
	const auto __integer = __base::__to_int();
	using _MaskType = decltype(__integer);

	constexpr auto __unused_bits = raze_sizeof_in_bits(_MaskType) - __base::__bit_width();

	if constexpr (__has_avx2_support_v<__base::__isa()>)
		return math::__lzcnt_clz(__integer) - __unused_bits;
	else
		return math::__bsr_clz(__integer) - __unused_bits;
}

template <class _Derived_>
constexpr uint8 __simd_element_mask_operations<_Derived_>::count_trailing_one_bits() const noexcept {
	const auto __integer = __base::__to_int();
	using _MaskType = decltype(__integer);

	constexpr auto __unused_bits = raze_sizeof_in_bits(_MaskType) - __base::__bit_width();

	if constexpr (__has_avx2_support_v<__base::__isa()>)
		return math::__tzcnt_ctz(static_cast<std::make_unsigned_t<_MaskType>>(~__integer));
	else
		return math::__bsf_ctz(static_cast<std::make_unsigned_t<_MaskType>>(~__integer));
}

template <class _Derived_>
constexpr uint8 __simd_element_mask_operations<_Derived_>::count_leading_one_bits() const noexcept {
	const auto __integer = __base::__to_int();
	using _MaskType = decltype(__integer);

	constexpr auto __unused_bits = raze_sizeof_in_bits(_MaskType) - __base::__bit_width();

	if constexpr (__has_avx2_support_v<__base::__isa()>)
		return math::__lzcnt_clz(static_cast<std::make_unsigned_t<_MaskType>>(~__integer)) - __unused_bits;
	else
		return math::__bsr_clz(static_cast<std::make_unsigned_t<_MaskType>>(~__integer)) - __unused_bits;
}

__RAZE_DATAPAR_NAMESPACE_END
