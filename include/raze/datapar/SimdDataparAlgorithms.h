#pragma once 

#include <raze/datapar/Simd.h>
#include <raze/datapar/SimdIndexMask.h>
#include <raze/datapar/BasicSimdMask.h>
#include <src/raze/datapar/Reduce.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <class _MaskType_>
__simd_nodiscard_inline auto __data(const _MaskType_& __mask) noexcept
	requires(__is_simd_mask_v<_MaskType_>)
{
	using _InternalMaskType = typename _MaskType_::mask_type;
	const auto __unwrapped_mask = static_cast<_InternalMaskType>(__mask);

	using _UnwrappedMaskType = decltype(__unwrapped_mask);

	if constexpr (__is_valid_simd_v<_UnwrappedMaskType>)
		return __data(__unwrapped_mask);
	else
		return __unwrapped_mask;
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
			std::remove_cvref_t<_DataparType_>, std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType	= std::remove_cvref_t<_DataparType_>;
	using _RawReductionType = std::remove_cvref_t<_ReduceBinaryFunction_>;

	if constexpr (type_traits::is_any_of_v<_RawReductionType, std::plus<>, type_traits::plus<>>)
		return _Simd_reduce_add<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type>()(__data(__datapar));
	else
		return _Simd_fold<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type>()(__data(__datapar), type_traits::__pass_function(__reduce));
}

template <class _DataparType_>
using __tail_mask_type = simd_mask<_DataparType_::__isa, typename _DataparType_::value_type, _DataparType_::__width>;

template <class _DataparType_> 
__simd_nodiscard_inline __tail_mask_type<_DataparType_> make_tail_mask(uint32 __bytes) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return __tail_mask_type<_DataparType_>(
		_Simd_make_tail_mask<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(__bytes));
}

/**
 *  @brief  Per‑lane absolute value.
 *
 *  @param __datapar  SIMD vector whose lanes are transformed.
 *
 *  @return  A SIMD vector where each lane contains the absolute value of the
 *           corresponding lane of @p __datapar.
 */
template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ abs(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_abs<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar));
}


/**
 *  @brief  Computes the horizontal minimum of all lanes.
 *
 *  @param __datapar  A SIMD vector.
 *
 *  @return  The minimum value among all lanes of @p __datapar.
 *
 *  The reduction order is unspecified. Equivalent to applying @c min()
 *  across all elements of the vector.
 */
template <class _DataparType_>
__simd_nodiscard_inline typename _DataparType_::value_type horizontal_min(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_horizontal_min<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar));
}

/**
 *  @brief  Computes the horizontal maximum of all lanes.
 *
 *  @param __datapar  A SIMD vector.
 *
 *  @return  The maximum value among all lanes of @p __datapar.
 *
 *  The reduction order is unspecified. Equivalent to applying @c max()
 *  across all elements of the vector.
 */
template <class _DataparType_>
__simd_nodiscard_inline typename _DataparType_::value_type horizontal_max(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_horizontal_max<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar));
}

/**
 *  @brief  Per‑lane minimum of two SIMD vectors.
 *
 *  @param __first   First SIMD vector.
 *  @param __second  Second SIMD vector.
 *
 *  @return  A SIMD vector whose lanes contain the element‑wise minimum of
 *           @p __first and @p __second.
 */ 
template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ vertical_min(
	const _DataparType_& __first, 
	const _DataparType_& __second) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_vertical_min<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__first), __data(__second));
}

/**
 *  @brief  Per‑lane maximum of two SIMD vectors.
 *
 *  @param __first   First SIMD vector.
 *  @param __second  Second SIMD vector.
 *
 *  @return  A SIMD vector whose lanes contain the element‑wise maximum of
 *           @p __first and @p __second.
 */
template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ vertical_max(
	const _DataparType_& __first,
	const _DataparType_& __second) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_vertical_max<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__first), __data(__second));
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

/**
 *  @brief  RAII‑guard for zeroing upper YMM state on exit.
 *
 *  Creates a scope guard that invokes @c _mm256_zeroupper() on destruction
 *  when the target ISA requires clearing upper register state after using
 *  wide SIMD instructions. This helps avoid transition penalties between
 *  legacy SSE code and AVX/AVX2/AVX‑512 code paths.
 *
 *  The guard is non‑copyable and non‑movable.  Constructing it marks the
 *  current scope as requiring a zero‑upper on exit; leaving the scope
 *  automatically performs the cleanup when mandated by the ISA.
 */
template <class _DataparType_>
raze_always_inline __zero_upper_at_exit_guard<std::remove_cvref_t<_DataparType_>::__isa> make_guard() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>) 
{
	return __zero_upper_at_exit_guard<std::remove_cvref_t<_DataparType_>::__isa>();
}

/**
 *  @brief  Lane-wise conditional selection.
 *
 *  @param __first   Value chosen when the mask lane is true.
 *  @param __second  Value chosen when the mask lane is false.
 *  @param __mask    SIMD mask or vector convertible to mask.
 *
 *  @return  A SIMD vector where each lane is selected from @p __first or
 *           @p __second according to the corresponding lane of @p __mask.
 *
 *  Semantics:
 *    result[i] = __mask[i] ? __first[i] : __second[i].
 *
 *  The mask may be a SIMD vector or a SIMD mask;
 */
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
		typename _RawDataparType::value_type>()(__data(__first), __data(__second), __data(__mask));
}

/**
 *  @brief  Reverses the order of lanes in a SIMD vector.

 *  @param __datapar  SIMD vector to be reversed.
 *
 *  @return A SIMD vector with lane order reversed.
 */

template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ reverse(const _DataparType_& __datapar) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_reverse<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar));
}

/**
 *  @brief  Issues a streaming store fence.
 *
 *  Ensures that all preceding non‑temporal (streaming) stores are completed
 *  before any subsequent memory operations.
 */
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

/**
 *  @brief  Compresses lanes according to a mask.
 *
 *  @param __datapar  Input SIMD vector.
 *  @param __mask     SIMD mask selecting which lanes are kept.
 *
 *  @return  A pair {count, vector}, where @c count is the number of bytes
 *           corresponding to the selected lanes, and @c vector contains 
 *			 those lanes packed nto the lowest positions.
 *
 *  The order of surviving lanes is preserved.
 */
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
		typename _RawDataparType::value_type>()(__data(__datapar), __data(__mask));
}

/**
 *  @brief  Compressed store of selected lanes.
 *
 *  @param __address  Destination address.
 *  @param __datapar  SIMD vector providing source values.
 *  @param __mask     Mask selecting which lanes to store.
 *
 *  @return Pointer to the element immediately after the last stored value.
 *
 *  Stores only the lanes where @p __mask is true, packing them contiguously
 *  starting at @p __address while preserving their original order.
 */
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
		reinterpret_cast<typename _RawDataparType::value_type*>(__address), __data(__mask), __data(__datapar), __policy);
}

/**
 *  @brief  Non‑temporal load.
 *
 *  @param __address  Pointer to the source memory.
 *
 *  @return  A SIMD vector loaded from @p __address using non‑temporal semantics.
 *
 *  Performs a full‑width load that hints to the processor that the data is
 *  not expected to be reused soon and therefore should bypass or minimize
 *  pollution of the cache hierarchy.
 */
template <class _DataparType_>
__simd_nodiscard_inline _DataparType_ non_temporal_load(const void* __address) noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_non_temporal_load<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::vector_type>()(__address);
}

/**
 *  @brief  Non‑temporal store.
 *
 *  @param __address  Pointer to the destination memory.
 *  @param __datapar  SIMD vector whose lanes are written.
 *
 *  Stores all lanes of @p __datapar to @p __address using non‑temporal semantics, 
 *  providing a hint that the stored data is not expected to be read again soon
 *  and should avoid polluting the cache hierarchy.
 */
template <class _DataparType_>
raze_always_inline void non_temporal_store(
	void*					__address,
	const _DataparType_&	__datapar) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	_Simd_non_temporal_store<_RawDataparType::__isa, _RawDataparType::__width>()(__address, __data(__datapar));
}

/**
 *  @brief  Loads a SIMD vector from memory.
 *
 *  @param __address  Pointer to the source memory.
 *
 *  @return  A SIMD vector containing all lanes loaded from @p __address,
 *           using the specified alignment policy.
 *
 *  Performs an unconditional load of the entire vector.
 */
template <
	class _DataparType_, 
	class _AlignmentPolicy_ = unaligned_policy>
raze_always_inline _DataparType_ load(
	const void*			__address,
	_AlignmentPolicy_&& __policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_load<_RawDataparType::__isa, _RawDataparType::__width, 
		typename _RawDataparType::vector_type>()(__address, __policy);
}

/**
 *  @brief  Stores a SIMD vector to memory.
 *
 *  @param __address  Pointer to the destination memory.
 *  @param __datapar  SIMD vector whose lanes are written to memory.
 *
 *  Stores all lanes of @p __datapar to @p __address using the specified alignment policy.
 */
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
	return _Simd_store<_RawDataparType::__isa, _RawDataparType::__width>()(__address, __data(__datapar), __policy);
}

/**
 *  @brief  Masked load with zeroing.
 *
 *  @param __address  Pointer to memory to load from.
 *  @param __mask     SIMD mask controlling which lanes are loaded.
 *
 *  @return  A SIMD vector where each lane is loaded from @p __address when
 *           the corresponding mask lane is true, and zero otherwise.
 *
 *  This operation may be used for tail handling only when
 *  @c _DataparType_::is_native_mask_load_supported is true, ensuring that
 *  masked-off lanes do not perform memory accesses. This form can be faster 
 *	than @c mask_load because masked-off lanes require no fallback value.
 */
template <
	class _DataparType_,
	class _MaskType_,
	class _AlignmentPolicy_ = unaligned_policy>
__simd_nodiscard_inline _DataparType_ maskz_load(
	const void*				__address,
	const _MaskType_&		__mask,
	_AlignmentPolicy_&&		__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> && (__is_simd_mask_v<std::remove_cvref_t<_MaskType_>>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;

	if constexpr (__is_valid_simd_v<typename _MaskType_::mask_type>) {
		return _Simd_maskz_load<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type, typename _RawDataparType::vector_type>()(reinterpret_cast<
				const typename _RawDataparType::value_type*>(__address), __data(__mask), __policy);
	}
	else {
		return _Simd_maskz_load<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type, typename _RawDataparType::vector_type>()(reinterpret_cast<
				const typename _RawDataparType::value_type*>(__address), __data(__mask), __policy);
	}
}

/**
 *  @brief  Masked load with fallback.
 *
 *  @param __address           Pointer to memory to load from.
 *  @param __mask              SIMD mask controlling which lanes are loaded.
 *  @param __additional_source Value used for lanes where @p __mask is false.
 *
 *  @return  A SIMD vector where each lane is loaded from @p __address when
 *           the corresponding mask lane is true, or taken from
 *           @p __additional_source otherwise.
 *
 *  This operation may be used for tail handling only when
 *  @c _DataparType_::is_native_mask_load_supported is true, ensuring that
 *  masked-off lanes do not perform memory accesses.
 */
template <
	class _DataparType_,
	class _MaskType_,
	class _AlignmentPolicy_ = unaligned_policy>
__simd_nodiscard_inline _DataparType_ mask_load(
	const void*					__address,
	const _MaskType_&			__mask,
	const _DataparType_&		__additional_source,
	_AlignmentPolicy_&&			__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> && __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Simd_mask_load<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		reinterpret_cast<const typename _RawDataparType::value_type*>(__address),
		__data(__mask), __data(__additional_source), __policy);
}

/**
 *  @brief  Masked store.
 *
 *  @param __address  Pointer to memory where elements are stored.
 *  @param __datapar  SIMD vector providing values to store.
 *  @param __mask     SIMD mask selecting which lanes are written.
 *
 *  Stores each lane of @p __datapar to @p __address when the corresponding
 *  lane of @p __mask is true; masked‑off lanes are not written. This
 *  operation may be used for tail handling only when
 *  @c _DataparType_::is_native_mask_store_supported is true, ensuring that
 *  masked‑off lanes do not perform memory accesses.
 */
template <
	class _DataparType_,	
	class _MaskType_,
	class _AlignmentPolicy_ = unaligned_policy>
raze_always_inline void mask_store(
	void*					__address,
	const _DataparType_&	__datapar,
	const _MaskType_&		__mask,
	_AlignmentPolicy_&&		__policy = _AlignmentPolicy_{}) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> && __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>)
{ 
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	_Simd_mask_store<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		reinterpret_cast<typename _RawDataparType::value_type*>(__address), __data(__mask), __data(__datapar), __policy);
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

	if constexpr (__is_native_compare_returns_number_v<_RawDataparType>)
		return reduce((__first == __second), type_traits::plus<>{});
	else
		return reduce(_IntDatapar::zero() - simd_cast<_ReduceType>((__first == __second)), type_traits::plus<>{});

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

	if constexpr (__is_native_compare_returns_number_v<_RawDataparType>)
		return reduce(((__first == __second) & __tail_mask), type_traits::plus<>{});
	else
		return reduce(_IntDatapar::zero() - simd_cast<_ReduceType>(((__first == __second) & __tail_mask)), type_traits::plus<>{});
}

template <class _SimdMask_>
__simd_nodiscard_inline bool none_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_SimdMask_>>)
{
	return __mask.__none_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline bool all_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_SimdMask_>>)
{
	return __mask.__all_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline bool any_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_SimdMask_>>)
{
	return __mask.__any_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline bool some_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<std::remove_cvref_t<_SimdMask_>>)
{
	return __mask.__some_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_first_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_>)
{
	return __mask.__count_trailing_zero_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_last_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_>)
{
	return __mask.__count_leading_zero_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_first_not_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_>)
{
	return __mask.__count_trailing_one_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_last_not_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_>)
{
	return __mask.__count_leading_one_bits();
}

template <class _SimdMaskType_>
__simd_nodiscard_inline auto count_set(const _SimdMaskType_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMaskType_>)
{
	return __mask.__count_set();
}


__RAZE_DATAPAR_NAMESPACE_END
