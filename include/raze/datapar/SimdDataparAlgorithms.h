#pragma once 

#include <raze/datapar/Simd.h>
#include <raze/datapar/SimdBitmask.h>
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
		return _Reduce_add<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type>()(__data(__datapar));
	else
		return _Fold<_RawDataparType::__isa, _RawDataparType::__width,
			typename _RawDataparType::value_type>()(__data(__datapar), type_traits::__pass_function(__reduce));
}

template <class _DataparType_>
using __tail_mask_type = simd_mask<typename _DataparType_::value_type, x86_abi<_DataparType_::__isa, _DataparType_::__width>>;

template <class _DataparType_> 
__simd_nodiscard_inline __tail_mask_type<_DataparType_> first_n(uint32 __elements) noexcept
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return __tail_mask_type<_DataparType_>(_First_n<_RawDataparType::__isa,
		_RawDataparType::__width, typename _RawDataparType::value_type>()(__elements));
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
	return _Abs<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Horizontal_min<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Horizontal_max<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Vertical_min<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Vertical_max<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Blend<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Reverse<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar));
}

/**
 *  @brief  Issues a streaming store fence.
 *
 *  Ensures that all preceding non‑temporal (streaming) stores are completed
 *  before any subsequent memory operations.
 */
template <class _DataparType_>
raze_always_inline void nt_fence() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	return _Nt_fence<std::remove_cvref_t<_DataparType_>::__isa>()();
}

template <arch::ISA _ISA_>
raze_always_inline void nt_fence() noexcept {
	return _Nt_fence<_ISA_>()();
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
	return _Compress<_RawDataparType::__isa, _RawDataparType::__width,
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
	return _Compress_store<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
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
	return _Nt_load<_RawDataparType::__isa, _RawDataparType::__width,
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
	_Nt_store<_RawDataparType::__isa, _RawDataparType::__width>()(__address, __data(__datapar));
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
	return _Load<_RawDataparType::__isa, _RawDataparType::__width, 
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
	return _Store<_RawDataparType::__isa, _RawDataparType::__width>()(__address, __data(__datapar), __policy);
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
		requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> && __is_simd_mask_v<std::remove_cvref_t<_MaskType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;

	return _Maskz_load<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type, typename _RawDataparType::vector_type>()(__address, __data(__mask), __policy);
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
	return _Mask_load<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		__address, __data(__mask), __data(__additional_source), __policy);
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
	_Mask_store<_RawDataparType::__isa, _RawDataparType::__width, typename _RawDataparType::value_type>()(
		__address, __data(__mask), __data(__datapar), __policy);
}

template <class _SimdMask_>
__simd_nodiscard_inline bool none_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__none_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline bool all_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__all_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline bool any_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__any_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline bool some_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__some_of();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_first_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_trailing_zero_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_last_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_leading_zero_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_first_not_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_trailing_one_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline int32 find_last_not_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_leading_one_bits();
}

template <class _SimdMask_>
__simd_nodiscard_inline auto count_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_set();
}

template <class _SimdMask_>
__simd_nodiscard_inline void clear_left(_SimdMask_& __mask) noexcept 
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	__mask.__clear_left();
}

template <class _SimdMask_>
__simd_nodiscard_inline void clear_right(_SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	__mask.__clear_right();
}

template <class _Simd_>
class simd_counter {
	using __accumulator_type = std::conditional_t<
		__is_native_compare_returns_number_v<_Simd_>,
		uint64, simd<typename IntegerForSizeof<typename _Simd_::value_type>::Signed, typename _Simd_::abi_type>>;
public:
	using mask_type = typename _Simd_::mask_type;
	
	simd_counter() noexcept 
	{}

	raze_always_inline void add(mask_type __mask) noexcept {
		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			_accumulator += datapar::count_set(__mask);
		else
			_accumulator -= static_cast<__accumulator_type>(__data(__mask));
	}

	raze_always_inline void add(
		mask_type __mask, 
		mask_type __tail_mask) noexcept 
	{
		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			_accumulator += datapar::count_set(__mask & __tail_mask);
		else
			_accumulator -= static_cast<__accumulator_type>(__data(__mask & __tail_mask));
	}

	raze_nodiscard raze_always_inline uint64 finalize() noexcept {
		auto __result = uint64(0);

		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			__result = _accumulator;
		else
			__result = datapar::reduce(_accumulator);

		_accumulator = 0;
		return __result;
	}
	
	raze_nodiscard raze_always_inline static constexpr uint64 portion_size() noexcept {
		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			return math::__maximum_integral_limit<__accumulator_type>() * sizeof(_Simd_);
		else
			return math::__maximum_integral_limit<typename IntegerForSizeof<typename _Simd_::value_type>::Signed>() * sizeof(_Simd_);
	}
private:
	__accumulator_type _accumulator = 0;
};

__RAZE_DATAPAR_NAMESPACE_END
