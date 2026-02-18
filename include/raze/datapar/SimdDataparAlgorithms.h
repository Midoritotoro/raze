#pragma once 

#include <raze/datapar/Simd.h>
#include <src/raze/datapar/Reduce.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	class _DataparType_,
	class _ReduceBinaryFunction_ = type_traits::plus<>>
__simd_nodiscard_inline auto reduce(
	const _DataparType_&		__datapar,
	_ReduceBinaryFunction_&&	__reduce = _ReduceBinaryFunction_{}) noexcept requires (
		(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> ||
			__is_intrin_type_v<std::remove_cvref_t<_DataparType_>>) &&
		std::is_invocable_r_v<std::remove_cvref_t<_DataparType_>, _ReduceBinaryFunction_,
			std::remove_cvref_t<_DataparType_>, std::remove_cvref_t<_DataparType_>>
		)
{
	using _RawDataparType	= std::remove_cvref_t<_DataparType_>;
	using _RawReductionType = std::remove_cvref_t<_ReduceBinaryFunction_>;

	if constexpr (type_traits::is_any_of_v<_RawReductionType, std::plus<>, type_traits::plus<>>)
		return _Simd_reduce_add<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type>()(__simd_unwrap(__datapar));
	else
		return _Simd_fold<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type>()(__simd_unwrap(__datapar), type_traits::__pass_function(__reduce));
}

template <
	class _DataparType_,
	class _ReduceBinaryFunction_ = type_traits::plus<>>
__simd_nodiscard_inline auto reduce(
	const _DataparType_& __datapar,
	_ReduceBinaryFunction_&& __unused = _ReduceBinaryFunction_{}) noexcept requires (
		__is_simd_mask_v<std::remove_cvref_t<_DataparType_>> ||
		__is_simd_index_mask_v<std::remove_cvref_t<_DataparType_>>
	)
{
	return __datapar.count_set();
}

template <
    class _DataparType_,
    class _ReduceBinaryFunction_ = type_traits::plus<>>
__simd_nodiscard_inline auto reduce(
    const _DataparType_& __datapar,
	_ReduceBinaryFunction_&& __unused = _ReduceBinaryFunction_{}) noexcept requires (
        std::is_integral_v<std::remove_cvref_t<_DataparType_>> &&
        std::is_unsigned_v<std::remove_cvref_t<_DataparType_>>
    )
{
    return math::__population_count(__datapar);
}

template <
	class		_Simd_,
	typename	_ReturnType_>
using __make_tail_mask_return_type_helper = std::conditional_t<__is_intrin_type_v<_ReturnType_>,
	simd<_Simd_::__isa, typename _Simd_::value_type, _Simd_::__width>,
	simd_mask<_Simd_::__isa, typename _Simd_::value_type, _Simd_::__width>>;

template <class _Simd_>
using __make_tail_mask_return_type = __make_tail_mask_return_type_helper<_Simd_,
	type_traits::invoke_result_type<_Simd_make_tail_mask<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>, uint32>>;

template <class _DataparType_> 
__simd_nodiscard_inline __make_tail_mask_return_type<_DataparType_> make_tail_mask(uint32 __bytes) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_make_tail_mask<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(__bytes);
}

template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ abs(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_abs<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar));
}


template <class _DataparType_>
__simd_nodiscard_inline typename _DataparType_::value_type horizontal_min(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_horizontal_min<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar));
}

template <class _DataparType_>
__simd_nodiscard_inline typename _DataparType_::value_type horizontal_max(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_horizontal_max<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar));
}

template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ vertical_min(
	const _DataparType_& __first, 
	const _DataparType_& __second) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_vertical_min<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__first), __simd_unwrap(__second));
}

template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ vertical_max(
	const _DataparType_& __first,
	const _DataparType_& __second) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_vertical_max<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__first), __simd_unwrap(__second));
}

template <arch::ISA _ISA_>
struct __zero_upper_at_exit_guard {
    __zero_upper_at_exit_guard(const __zero_upper_at_exit_guard&) noexcept = delete;
    __zero_upper_at_exit_guard(__zero_upper_at_exit_guard&&) noexcept = delete;

	__zero_upper_at_exit_guard() noexcept
	{}

	~__zero_upper_at_exit_guard() noexcept {
		if constexpr (type_traits::__is_zeroupper_required_v<_ISA_>)
			_mm256_zeroupper();
	}
};

template <arch::ISA _ISA_>
raze_always_inline __zero_upper_at_exit_guard<_ISA_> make_guard() noexcept {
	return __zero_upper_at_exit_guard<_ISA_>();
}

template <class _DataparType_>
raze_always_inline __zero_upper_at_exit_guard<std::remove_cvref_t<_DataparType_>::__isa> make_guard() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	return __zero_upper_at_exit_guard<std::remove_cvref_t<_DataparType_>::__isa>();
}

template <
	class _DataparType_,
	class _MaskType_>
__simd_nodiscard_inline _DataparType_ blend(
	const _DataparType_&	__first,
	const _DataparType_&	__second,
	const _MaskType_&		__mask) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> &&
			(__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_blend<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__first), __simd_unwrap(__second), __simd_unwrap_mask(__mask));
}

template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ reverse(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_reverse<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar));
}

template <class _DataparType_>
raze_always_inline void streaming_fence() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	return _Simd_streaming_fence<std::remove_cvref_t<_DataparType_>::__isa>()();
}

template <arch::ISA _ISA_>
raze_always_inline void streaming_fence() noexcept {
	return _Simd_streaming_fence<_ISA_>()();
}

template <
	class _DataparType_,
	class _MaskType_>
__simd_nodiscard_inline std::pair<uint32, _DataparType_> compress(
	const _DataparType_&	__datapar, 
	const _MaskType_&		__mask) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> &&
			(__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_compress<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar), __simd_unwrap_mask(__mask));
}

template <
	class _DataparType_,
    class _MaskType_,
    class _AlignmentPolicy_ = unaligned_policy>
raze_always_inline typename _DataparType_::value_type* compress_store(
	void*					__address,
	const _DataparType_&	__datapar,
	const _MaskType_&		__mask,
	_AlignmentPolicy_&&		__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> &&
			(__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_compress_store<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		reinterpret_cast<typename _RawDataparType::value_type*>(__address),
		__simd_unwrap_mask(__mask), __simd_unwrap(__datapar), __policy);
}

template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ non_temporal_load(const void* __address) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_non_temporal_load<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::vector_type>()(__address);
}

template <class _DataparType_>
raze_always_inline void non_temporal_store(
	void*					__address,
	const _DataparType_&	__datapar) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	_Simd_non_temporal_store<_RawDataparType::__isa, _RawDataparType::__width>()(__address, __simd_unwrap(__datapar));
}

template <
	class _DataparType_, 
	class _AlignmentPolicy_ = unaligned_policy>
raze_always_inline _DataparType_ load(
	const void*			__address,
	_AlignmentPolicy_&& __policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_load<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::vector_type>()(__address, __policy);
}

template <
	class _DataparType_, 
	class _AlignmentPolicy_ = unaligned_policy>
raze_always_inline void store(
	void*					__address,
	const _DataparType_&	__datapar,
	_AlignmentPolicy_&&		__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_store<_RawDataparType::__isa, _RawDataparType::__width>()(__address, __simd_unwrap(__datapar), __policy);
}

template <
	class _DataparType_,
	class _MaskType_,
	class _AlignmentPolicy_ = unaligned_policy>
__simd_nodiscard_inline _DataparType_ maskz_load(
	const void*				__address,
	const _MaskType_&		__mask,
	_AlignmentPolicy_&&		__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> &&
			(__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_maskz_load<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type, typename _RawDataparType::vector_type>()(reinterpret_cast<
			const typename _RawDataparType::value_type*>(__address), __simd_unwrap_mask(__mask), __policy);
}

template <
	class _DataparType_,
	class _MaskType_,
	class _AlignmentPolicy_ = unaligned_policy>
__simd_nodiscard_inline _DataparType_ mask_load(
	const void*					__address,
	const _MaskType_&			__mask,
	const _DataparType_&		__additional_source,
	_AlignmentPolicy_&&			__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> &&
			(__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_mask_load<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		reinterpret_cast<const typename _RawDataparType::value_type*>(__address),
		__simd_unwrap_mask(__mask), __simd_unwrap(__additional_source), __policy);
}

template <
	class _DataparType_,	
	class _MaskType_,
	class _AlignmentPolicy_ = unaligned_policy>
raze_always_inline void mask_store(
	void*					__address,
	const _DataparType_&	__datapar,
	const _MaskType_&		__mask,
	_AlignmentPolicy_&&		__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> &&
			(__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{ 
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	_Simd_mask_store<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		reinterpret_cast<typename _RawDataparType::value_type*>(__address),
		__simd_unwrap_mask(__mask), __simd_unwrap(__datapar), __policy);
}

template <class _DataparType_>
__simd_nodiscard_inline auto reduce_equal(
	const _DataparType_& __first,
	const _DataparType_& __second) noexcept
{
	using _ValueType	= typename _DataparType_::value_type;
	using _ReduceType	= typename IntegerForSizeof<_ValueType>::Signed;
	using _IntDatapar	= __rebind_vector_element_type<_ReduceType, _DataparType_>;

	constexpr auto __is_native_compare_return_number = std::is_integral_v<
		datapar::__simd_native_compare_return_type<_DataparType_, _ValueType>>;

	if constexpr (__is_native_compare_return_number)
		return reduce((__first == __second) | as_index_mask, type_traits::plus<>{});
	else
		return reduce(_IntDatapar::zero() - simd_cast<_ReduceType>((__first == __second) | as_simd), type_traits::plus<>{});

}

template <
	class _DataparType_,
	class _MaskType_>
__simd_nodiscard_inline auto reduce_equal(
	const _DataparType_&	__first,
	const _DataparType_&	__second,
	const _MaskType_&		__tail_mask) noexcept
{
	using _ValueType	= typename _DataparType_::value_type;
	using _ReduceType	= typename IntegerForSizeof<_ValueType>::Signed;
	using _IntDatapar	= __rebind_vector_element_type<_ReduceType, _DataparType_>;

	constexpr auto __is_native_compare_return_number = std::is_integral_v<
		datapar::__simd_native_compare_return_type<_DataparType_, _ValueType>>;

	if constexpr (__is_native_compare_return_number)
		return reduce(((__first == __second) & __tail_mask) | as_index_mask, type_traits::plus<>{});
	else
		return reduce(_IntDatapar::zero() - simd_cast<_ReduceType>(((__first == __second) & __tail_mask) | as_simd), type_traits::plus<>{});
}

__RAZE_DATAPAR_NAMESPACE_END
