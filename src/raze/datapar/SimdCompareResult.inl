__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	arch::ISA	_ISA_,
	class		_Type_,
	uint32		_Width_>
simd_compare_result<_ISA_, _Type_, _Width_>::simd_compare_result(const native_type& __result) noexcept :
	_compare_result(__result)
{}

template <
	arch::ISA	_ISA_,
	class		_Type_,
	uint32		_Width_>
simd_compare_result<_ISA_, _Type_, _Width_>::operator bool() const noexcept {
	return static_cast<bool>(*this | as_index_mask);
}

template <
	arch::ISA	_ISA_,
	class		_Type_,
	uint32		_Width_>
simd_compare_result<_ISA_, _Type_, _Width_> simd_compare_result<_ISA_, _Type_, _Width_>::operator~() noexcept
{
	return ~_compare_result;
}


__RAZE_DATAPAR_NAMESPACE_END
