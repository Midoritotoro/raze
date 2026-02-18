__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _CompareResult_>
__as_mask_t::__simd_mask_type<_CompareResult_> __as_mask_t::operator()(_CompareResult_&& __compare_result) const noexcept {
	using _WithoutReference_ = std::remove_cvref_t<_CompareResult_>;
	return _Simd_to_mask<_WithoutReference_::__isa, _WithoutReference_::__width,
		typename _WithoutReference_::element_type>()(__simd_unwrap_mask(__compare_result._compare_result));
}

template <class _CompareResult_>
__as_index_mask_t::__simd_index_mask_type<_CompareResult_> __as_index_mask_t::operator()(_CompareResult_&& __compare_result) const noexcept {
	using _WithoutReference_ = std::remove_cvref_t<_CompareResult_>;
	return static_cast<typename __simd_index_mask_type<_CompareResult_>::mask_type>(
		_Simd_to_index_mask<_WithoutReference_::__isa, _WithoutReference_::__width,
		typename _WithoutReference_::element_type>()(__simd_unwrap_mask(__compare_result._compare_result)));
}

template <class _CompareResult_>
__as_simd_t::__simd_type<_CompareResult_> __as_simd_t::operator()(_CompareResult_&& __compare_result) const noexcept {
	using _WithoutReference_ = std::remove_cvref_t<_CompareResult_>;

	return _Simd_to_vector<_WithoutReference_::__isa, _WithoutReference_::__width, typename simd<_WithoutReference_::__isa,
		typename _WithoutReference_::element_type, _WithoutReference_::__width>::vector_type,
		typename _WithoutReference_::element_type>()(__simd_unwrap_mask(__compare_result._compare_result));
}

template <class _CompareResult_>
__as_native_t::__native_type<_CompareResult_> __as_native_t::operator()(_CompareResult_&& __compare_result) const noexcept {
	return __compare_result._compare_result;
}

__RAZE_DATAPAR_NAMESPACE_END