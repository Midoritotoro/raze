#pragma once 

#include <raze/datapar/Simd.h>
#include <raze/datapar/SimdBitmask.h>
#include <raze/datapar/BasicSimdMask.h>
#include <src/raze/datapar/Reduce.h>
#include <src/raze/datapar/Expression.h>


__RAZE_DATAPAR_NAMESPACE_BEGIN

/**
 *  @brief  Computes the horizontal sum of all lanes in a SIMD vector.
 *
 *  @tparam _DataparType_  SIMD vector type.
 *
 *  @param __datapar  Input SIMD vector whose lanes are to be summed.
 *
 *  @return  The sum of all lanes of @p __datapar. The return type matches
 *           the underlying scalar element type of the vector.
 *
 *  This function performs a horizontal reduction using addition, combining
 *  all lanes of the SIMD vector into a single scalar value.
*/
template <class _DataparType_>
__simd_nodiscard_inline auto reduce_add(const _DataparType_& __datapar) noexcept 
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>> ||
		__is_intrin_type_v<std::remove_cvref_t<_DataparType_>>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Reduce_add<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar));
}

template <class _DataparType_>
using __tail_mask_type = simd_mask<typename _DataparType_::value_type, x86_abi<_DataparType_::__isa, _DataparType_::__width>>;

/**
 *  @brief  Constructs a tail mask with the first @p __elements lanes set.
 *
 *  @tparam _DataparType_  SIMD vector type for which the mask is created.
 *
 *  @param __elements  Number of leading lanes to mark as valid.
*/
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
 *  @brief  RAII‑guard for zeroing the upper YMM register state on exit.
 *
 *  Creates a scope guard that invokes @c _mm256_zeroupper() on destruction
 *  when the target ISA requires clearing the upper halves of YMM registers
 *  after executing wide SIMD instructions. This prevents performance
 *  penalties associated with transitions between legacy SSE code paths and
 *  AVX/AVX2/AVX‑512 instructions.
 *
 *  The guard is non‑copyable and non‑movable. Constructing it marks the
 *  current scope as requiring a zero‑upper on exit; leaving the scope
 *  automatically performs the cleanup when mandated by the ISA.
 *
 *  On architectures that do not require or support @c _mm256_zeroupper()
 *  (including non‑x86 targets and x86 ISAs where the transition penalty
 *  does not apply), the guard performs no action and compiles down to a
 *  no‑op.
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
__simd_nodiscard_inline _DataparType_ select(
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
 *
 *  On architectures that do not require or support an explicit store
 *  memory fence, this function may compile down to a no‑op.
*/
template <class _DataparType_>
raze_always_inline void nt_sfence() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	return _Nt_sfence<std::remove_cvref_t<_DataparType_>::__isa>()();
}

template <arch::ISA _ISA_>
raze_always_inline void nt_sfence() noexcept {
	return _Nt_sfence<_ISA_>()();
}

/**
 *  @brief  Issues a streaming load fence.
 *
 *  Ensures that all preceding non‑temporal (streaming) loads are completed
 *  before any subsequent memory operations.
 * 
 *  On architectures that do not require or support an explicit load
 *  memory fence, this function may compile down to a no‑op.
*/
template <class _DataparType_>
raze_always_inline void nt_lfence() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	return _Nt_lfence<std::remove_cvref_t<_DataparType_>::__isa>()();
}

template <arch::ISA _ISA_>
raze_always_inline void nt_lfence() noexcept {
	return _Nt_lfence<_ISA_>()();
}

/**
 *  @brief  Issues a full memory fence.
 *
 *  Ensures that all preceding memory operations (loads and stores),
 *  including non‑temporal (streaming) accesses, become globally visible
 *  before any subsequent memory operations are allowed to execute.
 *
 *  This is a full memory barrier that orders:
 *    - all prior loads before all subsequent loads and stores,
 *    - all prior stores before all subsequent loads and stores.
 *
 *  Unlike @c nt_sfence(), which orders only non‑temporal stores, and
 *  @c nt_lfence(), which orders only loads, this fence enforces a
 *  bidirectional ordering constraint across all types of memory
 *  operations.
 *
 *  On architectures that do not require or support an explicit full
 *  memory fence, this function may compile down to a no‑op.
*/
template <class _DataparType_>
raze_always_inline void nt_mfence() noexcept
	requires(__is_valid_simd_v<std::remove_cvref_t<_DataparType_>>)
{
	return _Nt_mfence<std::remove_cvref_t<_DataparType_>::__isa>()();
}

template <arch::ISA _ISA_>
raze_always_inline void nt_mfence() noexcept {
	return _Nt_mfence<_ISA_>()();
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
__simd_nodiscard_inline _DataparType_ nt_load(const void* __address) noexcept
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
raze_always_inline void nt_store(
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

/**
 *  @brief  Element-wise logical left shift of a SIMD vector.
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Number of elements to shift left.
 *
 *  Performs an element-wise logical left shift of @p __datapar by
 *  @p __elements positions. Elements shifted out of the vector on the
 *  left are discarded; vacated positions on the right are filled with
 *  zero.
 *
 *  If @p __elements is greater than or equal to the number of lanes in
 *  the vector, the result is a zero-filled vector.
*/
template <class _DataparType_>
raze_always_inline _DataparType_ slide_left(
    const _DataparType_&    __datapar,
    uint32                  __elements) noexcept
        requires(__is_valid_simd_v<_DataparType_>)
{
    using _RawDataparType = std::remove_cvref_t<_DataparType_>;
    return _Slide_left<_RawDataparType::__isa, _RawDataparType::__width>()(
        __data(__datapar), __elements * sizeof(typename _RawDataparType::value_type));
}

/**
 *  @brief  Element-wise logical left shift of a SIMD vector (compile-time constant).
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Compile-time constant number of elements to shift left.
 *
 *  Compile-time variant of slide_left(). Behaves identically to the
 *  runtime version, but allows the shift amount to be propagated as a
 *  compile-time constant to the backend implementation, enabling
 *  additional optimization opportunities.
*/
template <
    class   _DataparType_,
    uint32  _Elements_>
raze_always_inline _DataparType_ slide_left(
    const _DataparType_&                        __datapar,
    std::integral_constant<uint32, _Elements_>  __elements) noexcept
        requires(__is_valid_simd_v<_DataparType_>)
{
    using _RawDataparType = std::remove_cvref_t<_DataparType_>;
    return _Slide_left<_RawDataparType::__isa, _RawDataparType::__width>()(
        __data(__datapar), std::integral_constant<uint32,
            _Elements_ * sizeof(typename _RawDataparType::value_type)>{});
}

/**
 *  @brief  Element-wise logical right shift of a SIMD vector.
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Number of elements to shift right.
 *
 *  Performs an element-wise logical right shift of @p __datapar by
 *  @p __elements positions. Elements shifted out of the vector on the
 *  right are discarded; vacated positions on the left are filled with
 *  zero.
 *
 *  If @p __elements is greater than or equal to the number of lanes in
 *  the vector, the result is a zero-filled vector.
*/
template <class _DataparType_>
raze_always_inline _DataparType_ slide_right(
    const _DataparType_&    __datapar,
    uint32                  __elements) noexcept
        requires(__is_valid_simd_v<_DataparType_>)
{
    using _RawDataparType = std::remove_cvref_t<_DataparType_>;
    return _Slide_right<_RawDataparType::__isa, _RawDataparType::__width>()(
        __data(__datapar), __elements * sizeof(typename _RawDataparType::value_type));
}

/**
 *  @brief  Element-wise logical right shift of a SIMD vector (compile-time constant).
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Compile-time constant number of elements to shift right.
 *
 *  Compile-time variant of slide_right(). Behaves identically to the
 *  runtime version, but allows the shift amount to be propagated as a
 *  compile-time constant to the backend implementation, enabling
 *  additional optimization opportunities.
*/
template <
    class   _DataparType_,
    uint32  _Elements_>
raze_always_inline _DataparType_ slide_right(
    const _DataparType_&                        __datapar,
    std::integral_constant<uint32, _Elements_>  __elements) noexcept
        requires(__is_valid_simd_v<_DataparType_>)
{
    using _RawDataparType = std::remove_cvref_t<_DataparType_>;
    return _Slide_right<_RawDataparType::__isa, _RawDataparType::__width>()(
        __data(__datapar),
        std::integral_constant<uint32,
            _Elements_ * sizeof(typename _RawDataparType::value_type)>{});
}

/**
 *  @brief  Checks whether no lanes in the mask are set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return @c true if all lanes of @p __mask are false; otherwise @c false.
*/
template <class _SimdMask_>
__simd_nodiscard_inline bool none_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__none_of();
}

/**
 *  @brief  Checks whether all lanes in the mask are set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return @c true if every lane of @p __mask is true; otherwise @c false.
*/
template <class _SimdMask_>
__simd_nodiscard_inline bool all_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__all_of();
}

/**
 *  @brief  Checks whether at least one lane in the mask is set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return @c true if any lane of @p __mask is true; otherwise @c false.
*/
template <class _SimdMask_>
__simd_nodiscard_inline bool any_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__any_of();
}

/**
 *  @brief  Checks whether the mask is not fully set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return @c true if at least one lane of @p __mask is false; otherwise @c false.
 *
 *  This function is equivalent to:
 *  @code
 *      !all_of(__mask)
 *  @endcode
 *
*/
template <class _SimdMask_>
__simd_nodiscard_inline bool some_of(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__some_of();
}

/**
 *  @brief  Returns the index of the first set lane in the mask.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return The index of the lowest (least significant) lane that is set to true,
 *          or an mask elements count if no lanes are set.
*/
template <class _SimdMask_>
__simd_nodiscard_inline int32 find_first_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_trailing_zero_bits();
}

/**
 *  @brief  Returns the index of the last set lane in the mask.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return The index of the highest (most significant) lane that is set to true,
 *          or an mask elements count if no lanes are set.
 */
template <class _SimdMask_>
__simd_nodiscard_inline int32 find_last_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_leading_zero_bits();
}

/**
 *  @brief  Returns the index of the first lane that is not set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return The index of the lowest (least significant) lane that is false,
 *          or an mask elements count if all lanes are set.
*/
template <class _SimdMask_>
__simd_nodiscard_inline int32 find_first_not_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_trailing_one_bits();
}

/**
 *  @brief  Returns the index of the last lane that is not set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to inspect.
 *
 *  @return The index of the highest (most significant) lane that is false,
 *          or an mask elements count if all lanes are set.
*/
template <class _SimdMask_>
__simd_nodiscard_inline int32 find_last_not_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_leading_one_bits();
}

/**
 *  @brief  Returns the number of lanes set to true in the given SIMD mask.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask whose set bits are to be counted.
 *
 *  @return The number of true lanes in @p __mask.
*/
template <class _SimdMask_>
__simd_nodiscard_inline int32 count_set(const _SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__count_set();
}

/**
 *  @brief  Clears the highest set lane in the mask.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to modify.
 *
 *  Removes (sets to false) the most significant lane that is currently set
 *  in @p __mask. All other lanes remain unchanged.
*/
template <class _SimdMask_>
__simd_nodiscard_inline void clear_left(_SimdMask_& __mask) noexcept 
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	__mask.__clear_left();
}

/**
 *  @brief  Clears the lowest set lane in the mask.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to modify.
 *
 *  Removes (sets to false) the least significant lane that is currently set
 *  in @p __mask. All other lanes remain unchanged.
*/
template <class _SimdMask_>
__simd_nodiscard_inline void clear_right(_SimdMask_& __mask) noexcept
	requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	__mask.__clear_right();
}

/**
 *  @brief  Checks whether all mask lanes in the specified index range are set.
 *
 *  @tparam _SimdMask_  SIMD mask type or mask-bit type.
 *
 *  @param __mask  SIMD mask to be inspected.
 *  @param __n     Starting lane index (inclusive).
 *  @param __k     Ending lane index (inclusive).
 *
 *  @return @c true if all lanes in the range [@p __n, @p __k] are set to true;
 *          otherwise @c false.
 *
 *  This function verifies that the mask contains a contiguous block of set bits
 *  between indices @p __n and @p __k, inclusive.
 *
 *  @warning The behavior is undefined if:
 *           - @p __n or @p __k lie outside the valid lane range of @p __mask.
 *           - @p __n > @p __k.
 *
 *  This function does not perform bounds checking; callers must ensure that
 *  the provided indices are valid for the mask type.
*/
template <class _SimdMask_>
__simd_nodiscard_inline bool is_contiguous(
	const _SimdMask_& __mask,
	uint32             __n,
	uint32             __k) noexcept
		requires(__is_simd_mask_v<_SimdMask_> || __is_simd_mask_bits_v<_SimdMask_>)
{
	return __mask.__is_contiguous(__n, __k);
}

/**
 *  @brief  SIMD-accelerated counter for accumulating the number of true mask lanes.
 *
 *  @tparam _Simd_  SIMD vector type whose mask type is used for counting.
 *
 *  The @c simd_counter class provides mechanism for accumulating
 *  the number of true elements in SIMD comparison masks across large data
 *  ranges. It is designed for use inside vectorized loops where many SIMD
 *  comparisons are performed sequentially, and the total number of matches
 *  must be accumulated without overflow or loss of precision.
*/
template <class _Simd_>
class simd_counter {
	using __accumulator_type = std::conditional_t<
		__is_native_compare_returns_number_v<_Simd_>,
		uint64, simd<typename IntegerForSizeof<typename _Simd_::value_type>::Signed, typename _Simd_::abi_type>>;
public:
	using mask_type = typename _Simd_::mask_type;
	
	/**
     *  @brief  Constructs an empty counter with zero accumulator.
    */
	simd_counter() noexcept 
	{}
	
	/**
     *  @brief  Adds the number of true lanes in the given mask.
     *
     *  @param __mask  SIMD mask whose true elements are to be counted.
	*/
	raze_always_inline void add(mask_type __mask) noexcept {
		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			_accumulator += datapar::count_set(__mask);
		else
			_accumulator -= static_cast<__accumulator_type>(__data(__mask));
	}

	/**
     *  @brief  Adds the number of true lanes in the masked intersection of two masks.
     *
     *  @param __mask       Primary SIMD comparison mask.
     *  @param __tail_mask  Mask restricting valid lanes (typically for tail handling).
     *
     *  Only lanes that are true in both @p __mask and @p __tail_mask are counted.
     *  This overload is used when processing a partial SIMD block at the end of
     *  an input range.
    */
	raze_always_inline void add(
		mask_type __mask, 
		mask_type __tail_mask) noexcept 
	{
		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			_accumulator += datapar::count_set(__mask & __tail_mask);
		else
			_accumulator -= static_cast<__accumulator_type>(__data(__mask & __tail_mask));
	}

	/**
     *  @brief  Finalizes accumulation and returns the total count.
     *
     *  @return The number of true mask lanes accumulated since the last call.
     *
     *  After returning the result, the internal accumulator is reset to zero.
    */
	raze_nodiscard raze_always_inline uint64 finalize() noexcept {
		auto __result = uint64(0);

		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			__result = _accumulator;
		else
			__result = datapar::reduce_add(_accumulator);

		_accumulator = 0;
		return __result;
	}
	
	/**
     *  @brief  Maximum safe number of bytes that can be processed in one portion.
     *
     *  @return The maximum number of bytes that can be accumulated before the
     *          internal accumulator may overflow.
     *
     *  Vectorized counting algorithms use this value to split the input into
     *  portions that are guaranteed not to overflow the accumulator, ensuring
     *  correctness even for very large input ranges.
    */
	raze_nodiscard raze_always_inline static constexpr uint64 portion_size() noexcept {
		if constexpr (__is_native_compare_returns_number_v<_Simd_>)
			return math::__maximum_integral_limit<__accumulator_type>() * sizeof(_Simd_);
		else
			return math::__maximum_integral_limit<typename IntegerForSizeof<typename _Simd_::value_type>::Signed>() * sizeof(_Simd_);
	}
private:
	__accumulator_type _accumulator = 0;
};

/**
 *  @brief  Element-wise cyclic left rotation of a SIMD vector.
 *
 *  @tparam _DataparType_  SIMD vector type.
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Number of elements to rotate left.
 *
 *  @return A SIMD vector where the contents of @p __datapar have been rotated
 *          left by @p __elements positions. Elements shifted out on the left
 *          reappear at the right end of the vector.
 *
 *  This operation performs a cyclic rotation, unlike @c slide_left(), which
 *  fills vacated lanes with zero. For example:
 *
 *  @code
 *      [1 2 3 4] rotate_left by 1  →  [2 3 4 1]
 *  @endcode
*/
template <class _DataparType_>
raze_always_inline _DataparType_ rotate_left(
	const _DataparType_&	__datapar,
	uint32                  __elements) noexcept
		requires(__is_valid_simd_v<_DataparType_>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Rotate_left<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar), __elements);
}

/**
 *  @brief  Element-wise cyclic left rotation of a SIMD vector (compile-time constant).
 *
 *  @tparam _DataparType_  SIMD vector type.
 *  @tparam _Elements_     Compile-time constant rotation amount.
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Compile-time constant specifying the number of elements
 *                     to rotate left.
 *
 *  @return A SIMD vector rotated left by @p _Elements_ positions.
 *
 *  This overload behaves identically to the runtime version but allows the
 *  rotation amount to be propagated as a compile-time constant, enabling
 *  additional optimization opportunities in the backend.
*/
template <
	class   _DataparType_,
	uint32  _Elements_>
raze_always_inline _DataparType_ rotate_left(
	const _DataparType_&						__datapar,
	std::integral_constant<uint32, _Elements_>  __elements) noexcept
		requires(__is_valid_simd_v<_DataparType_>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Rotate_left<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar), __elements);
}

/**
 *  @brief  Element-wise cyclic right rotation of a SIMD vector.
 *
 *  @tparam _DataparType_  SIMD vector type.
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Number of elements to rotate right.
 *
 *  @return A SIMD vector where the contents of @p __datapar have been rotated
 *          right by @p __elements positions. Elements shifted out on the right
 *          reappear at the left end of the vector.
 *
 *  This operation performs a cyclic rotation, unlike @c slide_right(), which
 *  fills vacated lanes with zero. For example:
 *
 *  @code
 *      [1 2 3 4] rotate_right by 1  →  [4 1 2 3]
 *  @endcode
*/
template <class _DataparType_>
raze_always_inline _DataparType_ rotate_right(
	const _DataparType_&	__datapar,
	uint32                  __elements) noexcept
		requires(__is_valid_simd_v<_DataparType_>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Rotate_right<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar), __elements);
}

/**
 *  @brief  Element-wise cyclic right rotation of a SIMD vector (compile-time constant).
 *
 *  @tparam _DataparType_  SIMD vector type.
 *  @tparam _Elements_     Compile-time constant rotation amount.
 *
 *  @param __datapar   Input SIMD vector.
 *  @param __elements  Compile-time constant specifying the number of elements
 *                     to rotate right.
 *
 *  @return A SIMD vector rotated right by @p _Elements_ positions.
 *
 *  This overload behaves identically to the runtime version but allows the
 *  rotation amount to be propagated as a compile-time constant, enabling
 *  additional optimization opportunities in the backend.
*/
template <
	class   _DataparType_,
	uint32  _Elements_>
raze_always_inline _DataparType_ rotate_right(
	const _DataparType_&						__datapar,
	std::integral_constant<uint32, _Elements_>  __elements) noexcept
		requires(__is_valid_simd_v<_DataparType_>)
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Rotate_right<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar), __elements);
}

using prefetch_level = __prefetch_level;

/**
 * @brief Issues a prefetch hint for the specified memory address.
 *
 * @param __first  Iterator to the memory location to prefetch.
 * @param __level  Desired cache level or non-temporal hint.
 *
 * Provides a platform-independent interface for issuing hardware
 * prefetch instructions. The hint may improve performance when the
 * program will access @p __first in the near future.
 *
 * Semantics:
 *   - prefetch_level::L1  → data needed immediately
 *   - prefetch_level::L2  → data needed soon
 *   - prefetch_level::L3  → data needed later
 *   - prefetch_level::NTA → streaming access, avoid polluting caches
 *
 * The function is safe to call with any valid pointer. If the target
 * architecture does not support prefetching, the call becomes a no-op.
*/
template <
	class			_InputIterator_,
	prefetch_level	_Level_>
raze_always_inline void prefetch(
	_InputIterator_									__first,
	std::integral_constant<prefetch_level, _Level_>	__level) noexcept
		requires(type_traits::is_iterator_v<_InputIterator_> && 
			type_traits::is_iterator_input_ranges_v<_InputIterator_>)
{
	_Prefetch<__level>()(__first);
}

/**
 * @brief Performs a lane permutation of a SIMD vector using compile‑time
 *        known indices.
 *
 * This overload implements a shuffle operation where the permutation pattern
 * is encoded in the template parameter pack `_Indices_`. Although the shuffle
 * itself is executed at runtime, the backend can select the most efficient
 * ISA instructions at compile time because the indices are constant
 * expressions.
 *
 * @tparam _DataparType_
 *     A valid SIMD vector type (`simd<T, Abi>`).
 *
 * @tparam _Indices_
 *     A pack of compile‑time lane indices. The number of indices must be
 *     exactly `_DataparType_::size()`. Each index must be in the range
 *     `[0, _DataparType_::size())`. Out‑of‑range indices result in undefined
 *     behavior.
 *
 * @param __datapar
 *     The input SIMD vector whose lanes will be permuted.
 *
 * @param __indices
 *     A `std::integer_sequence<std::size_t, _Indices_...>` used for
 *     overload resolution.
 *
 * @return
 *     A new SIMD vector where lane `i` contains `__datapar[_Indices_[i]]`.
 *
 *     This often results in a single instruction or a minimal instruction
 *     sequence, unlike the runtime variant which must use general‑purpose
 *     dynamic permutation instructions.
*/
template <
	class		_DataparType_,
	uint64...	_Indices_>
raze_nodiscard raze_always_inline _DataparType_ shuffle(
	const _DataparType_&								__datapar,
	std::integer_sequence<std::size_t, _Indices_...>	__indices) noexcept
		requires(__is_valid_simd_v<_DataparType_> && sizeof...(_Indices_) == std::remove_cvref_t<_DataparType_>::size())
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Shuffle<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar), __indices);
}

/**
 * @brief Performs a lane permutation of a SIMD vector using runtime‑provided
 *        indices, enabling fully dynamic shuffle patterns.
 *
 * This overload implements a dynamic shuffle operation where the permutation
 * pattern is supplied at runtime via another SIMD vector. Each lane of
 * `__indices` specifies which lane of `__datapar` should be placed into the
 * corresponding lane of the result.
 *
 * @tparam _DataparType_
 *     A valid SIMD vector type (`simd<T, Abi>`).
 *
 * @tparam _IndexDataparType_
 *     A SIMD vector type whose element type is an integer and whose ABI
 *     matches `_DataparType_`. The number of lanes and ISA must be identical.
 *
 * @param __datapar
 *     The input SIMD vector to be permuted.
 *
 * @param __indices
 *     A SIMD vector of lane indices. For lane `i`, the value
 *     `__indices[i]` selects the source lane from `__datapar`. 
 *	   Out‑of‑range indices result in undefined behavior.
 *
 * @return
 *     A new SIMD vector where lane `i` contains `__datapar[__indices[i]]`.
*/
template <
	class _DataparType_,
	class _IndexDataparType_>
raze_nodiscard raze_always_inline _DataparType_ shuffle(
	const _DataparType_&		__datapar,
	const _IndexDataparType_&	__indices) noexcept
		requires(__is_valid_simd_v<_DataparType_> && __is_valid_simd_v<_IndexDataparType_> &&
			(std::remove_cvref_t<_DataparType_>::__isa == std::remove_cvref_t<_IndexDataparType_>::__isa) &&
			(std::remove_cvref_t<_DataparType_>::__width == std::remove_cvref_t<_IndexDataparType_>::__width) && 
			(std::remove_cvref_t<_DataparType_>::size() == std::remove_cvref_t<_IndexDataparType_>::size()) &&
			(std::is_integral_v<typename std::remove_cvref_t<_IndexDataparType_>::value_type>))
{
	using _RawDataparType = std::remove_cvref_t<_DataparType_>;
	return _Shuffle<_RawDataparType::__isa, _RawDataparType::__width,
		typename _RawDataparType::value_type>()(__data(__datapar), __data(__indices));
}

/**
 * @brief  Creates a masked expression object for conditional SIMD operations.
 *
 * Constructs a `_Where` expression that binds a SIMD vector, a mask, and an
 * additional source vector. The returned proxy object does not perform any
 * computation by itself; instead, it overloads arithmetic and bitwise operators
 * so that subsequent operations are executed under the control of @p __mask.
 *
 * @param __vector            The primary SIMD vector participating in masked operations.
 * @param __additional_source Fallback SIMD vector used for lanes where @p __mask is false.
 * @param __mask              SIMD mask controlling which lanes are active.
 *
 * @return A `_Where` proxy object that applies mask semantics to all supported
 *         operators. For each lane:
 *
 *         - if @p __mask[i] is true, the operator is applied to @p __vector[i],
 * 
 *         - otherwise, the result is taken from @p __additional_source[i].
 *
 * @note
 * This function does not perform any computation. It only constructs a lazy
 * expression object. The actual masked operation is executed when an operator
 * such as `+`, `-`, `*`, `&`, etc. is applied to the returned object.
 *
 * @see where(const _DataparType_&, const _MaskType_) for maskz semantics.
*/
template <
	class _DataparType_,
	class _MaskType_>
raze_nodiscard raze_always_inline _Where<_DataparType_, _MaskType_> where(
	_DataparType_&			__vector,
	const _DataparType_&	__additional_source,
	const _MaskType_&		__mask) noexcept
		requires(__is_valid_simd_v<_DataparType_> && __is_simd_mask_v<_MaskType_>)
{
	return _Where<_DataparType_, _MaskType_>(__vector, __additional_source, __mask);
}

/**
 * @brief  Creates a zero-masked expression object for conditional SIMD operations.
 *
 * Constructs a `_Where_zero` expression that binds a SIMD vector and a mask.
 * The returned proxy object overloads arithmetic and bitwise operators so that
 * subsequent operations are executed under the control of @p __mask.
 *
 * @param __vector  The SIMD vector participating in masked operations.
 * @param __mask    SIMD mask controlling which lanes are active.
 *
 * @return A `_Where_zero` proxy object that applies maskz semantics to all
 *         supported operators. For each lane:
 *
 *         - if @p __mask[i] is true, the operator is applied to @p __vector[i],
 * 
 *         - otherwise, the result is zero.
 *
 * @note
 * This function does not perform any computation. It only constructs a lazy
 * expression object. The actual masked operation is executed when an operator
 * such as `+`, `-`, `*`, `&`, etc. is applied to the returned object.
 *
 * @note
 * This overload corresponds to "maskz" semantics: inactive lanes are zeroed.
 *
 * @see where(const _DataparType_&, const _DataparType_&, const _MaskType_)
 *      for mask semantics with a fallback vector.
*/
template <
	class _DataparType_,
	class _MaskType_>
raze_nodiscard raze_always_inline _Where_zero<_DataparType_, _MaskType_> where(
	_DataparType_&		__vector,
	const _MaskType_&	__mask) noexcept
		requires(__is_valid_simd_v<_DataparType_> && __is_simd_mask_v<_MaskType_>)
{
	return _Where_zero<_DataparType_, _MaskType_>(__vector, __mask);
}

__RAZE_DATAPAR_NAMESPACE_END
