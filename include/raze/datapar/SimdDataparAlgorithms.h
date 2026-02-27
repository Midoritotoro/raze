#pragma once 

#include <raze/datapar/Simd.h>
#include <raze/datapar/SimdIndexMask.h>
#include <raze/datapar/BasicSimdMask.h>
#include <src/raze/datapar/Reduce.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _DataparType_>
__simd_nodiscard_inline auto to_index_mask(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	using _IndexMaskType = simd_index_mask<_RawDataparType::__isa, 
		typename _RawDataparType::value_type, _RawDataparType::__width>;

	return _IndexMaskType(
		_Simd_to_index_mask<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar)));
}

template <class _MaskType_>
__simd_nodiscard_inline auto to_index_mask(const _MaskType_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_MaskType_>>)
{
	using _RawType = std::remove_cvref_t<_MaskType_>;
	using _IndexMaskType = simd_index_mask<_RawType::__isa, typename _RawType::element_type, _RawType::__width>;
	using _VectorType = typename simd<_RawType::__isa, typename _RawType::element_type, _RawType::__width>::vector_type;

	const auto __vector = _Simd_to_vector<_RawType::__isa,
		_RawType::__width, _VectorType, typename _RawType::element_type>()(__simd_unwrap_mask(__mask));

	return _IndexMaskType(_Simd_to_index_mask<_RawType::__isa, _RawType::__width, typename _RawType::element_type>()(__vector));
}

template <class _IndexMaskType_>
__simd_nodiscard_inline auto to_index_mask(const _IndexMaskType_& __index_mask) noexcept
	requires(__is_simd_index_mask_v<std::remove_cvref_t<_IndexMaskType_>>)
{
	return __index_mask;
}

template <class _DataparType_>
__simd_nodiscard_inline auto to_mask(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	using _MaskType = simd_mask<_RawDataparType::__isa,
		typename _RawDataparType::value_type, _RawDataparType::__width>;

	return _MaskType(
		_Simd_to_mask<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__simd_unwrap(__datapar)));
}

template <class _MaskType_>
__simd_nodiscard_inline auto to_mask(const _MaskType_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_MaskType_>>)
{
	return __mask;
}

template <class _IndexMaskType_>
__simd_nodiscard_inline auto to_mask(const _IndexMaskType_& __index_mask) noexcept
	requires(__is_simd_index_mask_v<std::remove_cvref_t<_IndexMaskType_>>)
{
	using _RawType = std::remove_cvref_t<_IndexMaskType_>;
	using _VectorType = typename simd<_RawType::__isa, typename _RawType::value_type, _RawType::__width>::vector_type;
	using _MaskType = simd_mask<_RawType::__isa, typename _RawType::value_type, _RawType::__width>;

	const auto __vector = _Simd_index_mask_to_vector<_RawType::__isa,
		_RawType::__width, _VectorType, typename _RawType::value_type>()(__simd_unwrap_mask(__index_mask));

	return _MaskType(_Simd_to_mask<_RawType::__isa, _RawType::__width, typename _RawType::value_type>()(__vector));
}

template <class _DataparType_>
__simd_nodiscard_inline auto to_simd(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	return __datapar;
}

template <class _IndexMaskType_>
__simd_nodiscard_inline auto to_simd(const _IndexMaskType_& __index_mask) noexcept
	requires(__is_simd_index_mask_v<std::remove_cvref_t<_IndexMaskType_>>)
{
	using _RawType = std::remove_cvref_t<_IndexMaskType_>;
	using _VectorType = typename simd<_RawType::__isa, typename _RawType::value_type, _RawType::__width>::vector_type;
	using _SimdType = simd<_RawType::__isa, typename _RawType::element_type, _RawType::__width>;

	return _SimdType(_Simd_index_mask_to_vector<_RawType::__isa, _RawType::__width,
		_VectorType, typename _RawType::value_type>()(__simd_unwrap_mask(__index_mask)));
}

template <class _MaskType_>
__simd_nodiscard_inline auto to_simd(const _MaskType_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_MaskType_>>)
{
	using _RawType = std::remove_cvref_t<_MaskType_>;
	using _VectorType = typename simd<_RawType::__isa, typename _RawType::element_type, _RawType::__width>::vector_type;
	using _SimdType = simd<_RawType::__isa, typename _RawType::element_type, _RawType::__width>;

	return _SimdType(_Simd_to_vector<_RawType::__isa, _RawType::__width,
		_VectorType, typename _RawType::element_type>()(__simd_unwrap_mask(__mask)));
}

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

// Lane-wise select: result[i] = mask[i] ? first[i] : second[i].
template <
	class _DataparType_,
	class _MaskType_>
__simd_nodiscard_inline _DataparType_ blend(
	const _DataparType_&	__first,
	const _DataparType_&	__second,
	const _MaskType_&		__mask) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> 
			&& (__is_valid_simd_v<std::remove_cvref_t<_MaskType_>> || __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
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
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	using _ValueType = typename _RawDataparType::value_type;
	using _ReduceType = typename IntegerForSizeof<_ValueType>::Signed;
	using _IntDatapar = __rebind_vector_element_type<_ReduceType, _RawDataparType>;

	constexpr auto __is_native_compare_return_number = __is_simd_mask_v<
		typename _RawDataparType::__simd_native_compare_return_type>;

	if constexpr (__is_native_compare_return_number)
		return reduce(to_index_mask(__first == __second), type_traits::plus<>{});
	else
		return reduce(_IntDatapar::zero() - simd_cast<_ReduceType>(to_simd(__first == __second)), type_traits::plus<>{});

}

template <
	class _DataparType_,
	class _MaskType_>
__simd_nodiscard_inline auto reduce_equal(
	const _DataparType_&	__first,
	const _DataparType_&	__second,
	const _MaskType_&		__tail_mask) noexcept
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	using _ValueType = typename _RawDataparType::value_type;
	using _ReduceType = typename IntegerForSizeof<_ValueType>::Signed;
	using _IntDatapar = __rebind_vector_element_type<_ReduceType, _RawDataparType>;

	constexpr auto __is_native_compare_return_number = __is_simd_mask_v<
		typename _RawDataparType::__simd_native_compare_return_type>;

	if constexpr (__is_native_compare_return_number)
		return reduce(to_index_mask((__first == __second) & __tail_mask), type_traits::plus<>{});
	else
		return reduce(_IntDatapar::zero() - simd_cast<_ReduceType>(to_simd((__first == __second) & __tail_mask)), type_traits::plus<>{});
}

template <class _DataparType_>
__simd_nodiscard_inline bool none_of(const _DataparType_& __datapar) noexcept 
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_testz<_RawDataparType::__isa, _RawDataparType::__width>()(__simd_unwrap(__datapar));
}

template <class _SimdMask_>
__simd_nodiscard_inline bool none_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_SimdMask_>> || __is_simd_index_mask_v<std::remove_cvref_t<_SimdMask_>>)
{
	return __mask.none_of();
}

template <class _DataparType_>
__simd_nodiscard_inline bool all_of(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_test_all_ones<_RawDataparType::__isa, _RawDataparType::__width>()(__simd_unwrap(__datapar));
}

template <class _SimdMask_>
__simd_nodiscard_inline bool all_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_SimdMask_>> || __is_simd_index_mask_v<std::remove_cvref_t<_SimdMask_>>)
{
	return __mask.all_of();
}

template <class _DataparType_>
__simd_nodiscard_inline bool any_of(const _DataparType_& __datapar) noexcept requires(
	__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> || __is_simd_mask_v<std::remove_cvref_t<_DataparType_>>
		|| __is_simd_index_mask_v<std::remove_cvref_t<_DataparType_>>)
{
	return !none_of(__datapar);
}

template <class _DataparType_>
__simd_nodiscard_inline bool some_of(const _DataparType_& __datapar) noexcept requires(
	__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> || __is_simd_mask_v<std::remove_cvref_t<_DataparType_>>
		|| __is_simd_index_mask_v<std::remove_cvref_t<_DataparType_>>)
{
	return !all_of(__datapar);
}

__RAZE_DATAPAR_NAMESPACE_END
