#pragma once 

#include <raze/vx/Simd.h>
#include <raze/vx/SimdBitmask.h>
#include <raze/vx/BasicSimdMask.h>
#include <src/raze/vx/Reduce.h>
#include <src/raze/vx/Expression.h>
#include <raze/vx/Concepts.h>

__RAZE_VX_NAMESPACE_BEGIN

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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline auto reduce_add(const _Simd_& __vector) noexcept {
	return _Reduce_add<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector));
}

template <non_memory_where_expression_type _WhereExpression_>
raze_nodiscard raze_always_inline auto reduce_add(const _WhereExpression_& __where) noexcept {
	return __where.__reduce_add();
}

template <class _DataparType_>
using __tail_mask_type = simd_mask<typename _DataparType_::value_type, x86_runtime_abi<_DataparType_::__isa, _DataparType_::__width>>;

/**
 *  @brief  Constructs a tail mask with the first @p __elements lanes set.
 *
 *  @tparam _DataparType_  SIMD vector type for which the mask is created.
 *
 *  @param __elements  Number of leading lanes to mark as valid.
*/
template <simd_type _Simd_> 
raze_nodiscard raze_always_inline __tail_mask_type<_Simd_> 
	first_n(uint32 __elements) noexcept 
{
	return __tail_mask_type<_Simd_>(_First_n<_Simd_::__isa,
		_Simd_::__width, typename _Simd_::value_type>()(__elements));
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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline typename _Simd_::value_type 
	horizontal_min(const _Simd_& __vector) noexcept 
{
	return _Horizontal_min<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector));
}

template <non_memory_where_expression_type _WhereExpression_>
raze_nodiscard raze_always_inline typename _WhereExpression_::value_type	
	horizontal_min(const _WhereExpression_& __where) noexcept
{
	return __where.__horizontal_min();
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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline typename _Simd_::value_type
	horizontal_max(const _Simd_& __vector) noexcept
{
	return _Horizontal_max<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector));
}

template <non_memory_where_expression_type _WhereExpression_>
raze_nodiscard raze_always_inline typename _WhereExpression_::value_type	
	horizontal_max(const _WhereExpression_& __where) noexcept
{
	return __where.__horizontal_max();
}

/**
 *  @brief  Per‑lane minimum of two SIMD vectors.
 *
 *  @param __x  First SIMD vector.
 *  @param __y  Second SIMD vector.
 *
 *  @return  A SIMD vector whose lanes contain the element‑wise minimum of
 *           @p __first and @p __second.
 */ 
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ vertical_min(
	const _Simd_& __x,
	const _Simd_& __y) noexcept
{
	return _Vertical_min<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__x), __data(__y));
}

template <
	simd_type							_Simd_,
	non_memory_where_expression_type	_WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ vertical_min(
	const _Simd_&				__vector,
	const _WhereExpression_&	__where) noexcept
{
	return __where.__vertical_min(__vector);
}

template <
	non_memory_where_expression_type	_WhereExpression_,
	simd_type							_Simd_>
raze_nodiscard raze_always_inline _Simd_ vertical_min(
	const _WhereExpression_&	__where,
	const _Simd_&				__vector) noexcept
{
	return __where.__vertical_min(__vector);
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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ vertical_max(
	const _Simd_& __x,
	const _Simd_& __y) noexcept
{
	return _Vertical_max<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__x), __data(__y));
}

template <
	simd_type							_Simd_,
	non_memory_where_expression_type	_WhereExpression_>
raze_nodiscard raze_always_inline _Simd_ vertical_max(
	const _Simd_&				__vector,
	const _WhereExpression_&	__where) noexcept
{
	return __where.__vertical_max(__vector);
}

template <
	non_memory_where_expression_type	_WhereExpression_,
	simd_type							_Simd_>
raze_nodiscard raze_always_inline _Simd_ vertical_max(
	const _WhereExpression_&	__where,
	const _Simd_&				__vector) noexcept
{
	return __where.__vertical_max(__vector);
}

template <arch::ISA _ISA_>
struct __zero_upper_at_exit_guard {
    __zero_upper_at_exit_guard(const __zero_upper_at_exit_guard&) noexcept = delete;
    __zero_upper_at_exit_guard(__zero_upper_at_exit_guard&&) noexcept = delete;

	__zero_upper_at_exit_guard() noexcept
	{}

	~__zero_upper_at_exit_guard() noexcept {
#if defined(raze_processor_x86)
		if constexpr (type_traits::__is_zeroupper_required_v<_ISA_>)
			_mm256_zeroupper();
#endif
	}
};

template <arch::ISA _ISA_>
raze_always_inline auto make_guard() noexcept {
	return __zero_upper_at_exit_guard<_ISA_>{};
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
template <simd_type _Simd_>
raze_always_inline auto make_guard() noexcept {
	return __zero_upper_at_exit_guard<_Simd_::__isa>{};
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
	simd_type		_Simd_,
	simd_mask_type	_SimdMask_>
raze_nodiscard raze_always_inline _Simd_ select(
	const _Simd_&		__first,
	const _Simd_&		__second,
	const _SimdMask_&	__mask) noexcept
{
	return _Blend<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>()(
		__data(__first), __data(__second), __data(__mask));
}

/**
 *  @brief  Reverses the order of lanes in a SIMD vector.

 *  @param __datapar  SIMD vector to be reversed.
 *
 *  @return A SIMD vector with lane order reversed.
*/
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ reverse(const _Simd_& __x) noexcept {
	return _Reverse<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__x));
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
template <simd_type _Simd_>
raze_always_inline void nt_sfence() noexcept {
	return _Nt_sfence<_Simd_::__isa>()();
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
template <simd_type _Simd_>
raze_always_inline void nt_lfence() noexcept {
	return _Nt_lfence<_Simd_::__isa>()();
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
template <simd_type _Simd_>
raze_always_inline void nt_mfence() noexcept {
	return _Nt_mfence<_Simd_::__isa>()();
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
	simd_type		_Simd_,
	simd_mask_type	_SimdMask_>
__simd_nodiscard_inline std::pair<uint32, _Simd_> compress(
	const _Simd_&		__vector,
	const _SimdMask_&	__mask) noexcept
{
	return _Compress<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __data(__mask));
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
	std::input_or_output_iterator	_Iterator_,
	simd_type						_Simd_,
    simd_mask_type					_SimdMask_,
	alignment_policy_type			_AlignmentPolicy_ = unaligned_policy>
raze_always_inline typename _Simd_::value_type* compress_store(
	_Iterator_			__first,
	const _Simd_&		__vector,
	const _SimdMask_&	__mask,
	_AlignmentPolicy_&&	__policy = _AlignmentPolicy_{}) noexcept
{
	return _Compress_store<_Simd_::__isa, _Simd_::__width, typename _Simd_::value_type>()(
		std::to_address(__first), __data(__mask), __data(__vector), __policy);
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
template <
	std::input_or_output_iterator	_Iterator_,
	simd_type						_Simd_>
raze_nodiscard raze_always_inline _Simd_ nt_load(_Iterator_ __first) noexcept {
	return _Nt_load<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::vector_type>()(std::to_address(__first));
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
template <
	std::input_or_output_iterator	_Iterator_,
	simd_type						_Simd_>
raze_always_inline void nt_store(
	_Iterator_		__first,
	const _Simd_&	__vector) noexcept
{
	_Nt_store<_Simd_::__isa, _Simd_::__width>()(
		std::to_address(__first), __data(__vector));
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
	simd_type						_Simd_,
	std::input_or_output_iterator	_Iterator_,
	alignment_policy_type			_AlignmentPolicy_ = unaligned_policy>
raze_nodiscard raze_always_inline _Simd_ load(
	_Iterator_			__first,
	_AlignmentPolicy_&& __policy = _AlignmentPolicy_{}) noexcept
{
	return _Load<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::vector_type>()(std::to_address(__first), __policy);
}

template <
	where_memory_expression_type	_WhereMemoryExpression_, 
	alignment_policy_type			_AlignmentPolicy_ = unaligned_policy>
raze_always_inline typename _WhereMemoryExpression_::datapar_type load(
	const _WhereMemoryExpression_&	__where,
	_AlignmentPolicy_&&				__policy = _AlignmentPolicy_{}) noexcept
{
	return __where.__load(__policy);
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
	std::input_or_output_iterator	_Iterator_,
	simd_type						_Simd_, 
	alignment_policy_type			_AlignmentPolicy_ = unaligned_policy>
raze_always_inline void store(
	_Iterator_			__first,
	const _Simd_&		__vector,
	_AlignmentPolicy_&&	__policy = _AlignmentPolicy_{}) noexcept
{
	return _Store<_Simd_::__isa, _Simd_::__width>()(
		std::to_address(__first), __data(__vector), __policy);
}

template <
	std::input_or_output_iterator		_Iterator_,
	non_memory_where_expression_type	_WhereExpression_, 
	alignment_policy_type				_AlignmentPolicy_ = unaligned_policy>
raze_always_inline void store(
	_Iterator_					__first,
	const _WhereExpression_&	__where,
	_AlignmentPolicy_&&			__policy = _AlignmentPolicy_{}) noexcept
{
	__where.__store(__first, __policy);
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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ slide_left(
    const _Simd_&   __vector,
    uint32          __elements) noexcept
{
    return _Slide_left<_Simd_::__isa, _Simd_::__width>()(
        __data(__vector), __elements * sizeof(typename _Simd_::value_type));
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
	simd_type	_Simd_,
    uint32		_Elements_>
raze_nodiscard raze_always_inline _Simd_ slide_left(
    const _Simd_&								__vector,
    std::integral_constant<uint32, _Elements_>  __elements) noexcept
{
    return _Slide_left<_Simd_::__isa, _Simd_::__width>()(
        __data(__vector), std::integral_constant<uint32,
            _Elements_ * sizeof(typename _Simd_::value_type)>{});
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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ slide_right(
    const _Simd_&   __datapar,
    uint32			__elements) noexcept
{
    return _Slide_right<_Simd_::__isa, _Simd_::__width>()(
        __data(__datapar), __elements * sizeof(typename _Simd_::value_type));
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
    simd_type   _Simd_,
    uint32		_Elements_>
raze_nodiscard raze_always_inline _Simd_ slide_right(
    const _Simd_&								__vector,
    std::integral_constant<uint32, _Elements_>  __elements) noexcept
{
    return _Slide_right<_Simd_::__isa, _Simd_::__width>()(
        __data(__vector), std::integral_constant<uint32,
            _Elements_ * sizeof(typename _Simd_::value_type)>{});
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline bool none_of(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline bool all_of(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline bool any_of(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline bool some_of(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline int32 find_first_set(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline int32 find_last_set(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline int32 find_first_not_set(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline int32 find_last_not_set(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline int32 count_set(const _SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_always_inline void clear_left(_SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_always_inline void clear_right(_SimdMask_& __mask) noexcept {
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
template <simd_mask_type _SimdMask_>
raze_nodiscard raze_always_inline bool is_contiguous(
	const _SimdMask_&	__mask,
	uint32				__n,
	uint32				__k) noexcept
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
template <simd_type _Simd_>
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
			_accumulator += vx::count_set(__mask);
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
			_accumulator += vx::count_set(__mask & __tail_mask);
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
			__result = vx::reduce_add(_accumulator);

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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ rotate_left(
	const _Simd_&	__vector,
	uint32			__elements) noexcept
{
	return _Rotate_left<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __elements);
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
	simd_type   _Simd_,
	uint32		_Elements_>
raze_nodiscard raze_always_inline _Simd_ rotate_left(
	const _Simd_&								__vector,
	std::integral_constant<uint32, _Elements_>  __elements) noexcept
{
	return _Rotate_left<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __elements);
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
template <simd_type _Simd_>
raze_nodiscard raze_always_inline _Simd_ rotate_right(
	const _Simd_&	__vector,
	uint32          __elements) noexcept
{
	return _Rotate_right<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __elements);
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
	simd_type   _Simd_,
	uint32		_Elements_>
raze_nodiscard raze_always_inline _Simd_ rotate_right(
	const _Simd_&								__vector,
	std::integral_constant<uint32, _Elements_>  __elements) noexcept
{
	return _Rotate_right<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __elements);
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
	std::input_or_output_iterator	_Iterator_,
	prefetch_level					_Level_>
raze_always_inline void prefetch(
	_Iterator_										__first,
	std::integral_constant<prefetch_level, _Level_>	__level) noexcept
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
	simd_type		_Simd_,
	uint64...		_Indices_>
raze_nodiscard raze_always_inline _Simd_ shuffle(
	const _Simd_&										__vector,
	std::integer_sequence<std::size_t, _Indices_...>	__indices) noexcept
		requires(sizeof...(_Indices_) == _Simd_::size())
{
	return _Shuffle<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __indices);
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
	simd_type _Simd_,
	simd_type _IndexSimd_>
raze_nodiscard raze_always_inline _Simd_ shuffle(
	const _Simd_&		__vector,
	const _IndexSimd_&	__indices) noexcept
		requires((_Simd_::__isa == _IndexSimd_::__isa) && (_Simd_::__width == _IndexSimd_::__width) &&
			(_Simd_::size() == _IndexSimd_::size()) && (std::is_integral_v<typename _IndexSimd_::value_type>))
{
	return _Shuffle<_Simd_::__isa, _Simd_::__width,
		typename _Simd_::value_type>()(__data(__vector), __data(__indices));
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
 * @param __source Fallback SIMD vector used for lanes where @p __mask is false.
 * @param __mask              SIMD mask controlling which lanes are active.
 *
 * @return A `_Where` proxy object that applies mask semantics to all supported
 *         operators. For each lane:
 *
 *         - if @p __mask[i] is true, the operator is applied to @p __vector[i],
 * 
 *         - otherwise, the result is taken from @p __source[i].
 *
 * @note
 * This function does not perform any computation. It only constructs a lazy
 * expression object. The actual masked operation is executed when an operator
 * such as `+`, `-`, `*`, `&`, etc. is applied to the returned object.
 *
 * @see where(_DataparType_&, const _MaskType_) for maskz semantics.
*/
template <
	simd_type		_Simd_,
	simd_mask_type	_SimdMask_>
raze_nodiscard raze_always_inline _Where<_Simd_, _SimdMask_> where(
	_Simd_&				__vector,
	const _Simd_&		__source,
	const _SimdMask_&	__mask) noexcept
{
	return _Where<_Simd_, _SimdMask_>(__vector, __source, __mask);
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
	simd_type		_Simd_,
	simd_mask_type	_SimdMask_>
raze_nodiscard raze_always_inline _Where_zero<_Simd_, _SimdMask_> where(
	_Simd_&				__vector,
	const _SimdMask_&	__mask) noexcept
{
	return _Where_zero<_Simd_, _SimdMask_>(__vector, __mask);
}

template <
	simd_type		_Simd_,
	simd_mask_type	_SimdMask_>
raze_nodiscard raze_always_inline _Const_where<_Simd_, _SimdMask_> where(
	const _Simd_&		__vector,
	const _Simd_&		__source,
	const _SimdMask_&	__mask) noexcept
{
	return _Const_where<_Simd_, _SimdMask_>(__vector, __source, __mask);
}

template <
	simd_type		_Simd_,
	simd_mask_type	_SimdMask_>
raze_nodiscard raze_always_inline _Const_where_zero<_Simd_, _SimdMask_> where(
	const _Simd_&		__vector,
	const _SimdMask_&	__mask) noexcept
{
	return _Const_where_zero<_Simd_, _SimdMask_>(__vector, __mask);
}

template <
	std::input_or_output_iterator	_Iterator_,
	simd_type						_Simd_,
	simd_mask_type					_SimdMask_>
raze_nodiscard raze_always_inline _Where_memory<_Iterator_, _Simd_, _SimdMask_> where(
	_Iterator_			__first,
	const _Simd_&		__source,
	const _SimdMask_&	__mask) noexcept
{
	return _Where_memory<_Iterator_, _Simd_, _SimdMask_>(__first, __source, __mask);
}

template <
	std::input_or_output_iterator	_Iterator_,
	simd_mask_type					_SimdMask_>
raze_nodiscard raze_always_inline _Where_zero_memory<_Iterator_, _SimdMask_> where(
	_Iterator_			__first,
	const _SimdMask_&	__mask) noexcept
{
	return _Where_zero_memory<_Iterator_, _SimdMask_>(__first, __mask);
}

/**
 * @brief Applies a compile‑time ternary boolean function to three SIMD vectors.
 *
 * This function evaluates boolean operation defined by the
 * compile‑time constant @_TernaryMask_. The mask encodes the truth table of a
 * ternary boolean function and determines how the corresponding lanes of
 * @p __x, @p __y, and @p __z are combined.
 *
 * @tparam _Simd_
 *     A valid SIMD vector type (`simd<T, Abi>`).
 *
 * @tparam _TernaryMask_
 *     An 8‑bit compile‑time constant in the range [0, 255] describing the
 *     ternary boolean function to apply.
 *
 * @param __x  First SIMD input vector.
 * @param __y  Second SIMD input vector.
 * @param __z  Third SIMD input vector.
 * @param __imm8
 *     A `std::integral_constant<uint8, _TernaryMask_>` selecting the boolean
 *     function at compile time.
 *
 * @return
 *     A SIMD vector where each lane contains the result of applying the
 *     ternary boolean function to the corresponding lanes of the inputs.
 *
 * @example
 *     auto r = ternarylogic(a, b, c, std::integral_constant<uint8, 0x96>{});
 *
 * @example
 *     auto r = ternarylogic(x, y, z, as_ternary_mask<A & B | ~C>());
*/
template <
	simd_type	_Simd_,
	uint8		_TernaryMask_>
raze_nodiscard raze_always_inline _Simd_ ternarylogic(
	const _Simd_&									__x,
	const _Simd_&									__y,
	const _Simd_&									__z,
	std::integral_constant<uint8, _TernaryMask_>	__imm8) noexcept
		requires((_TernaryMask_ >= 0 && _TernaryMask_ <= 255) && 
			__is_valid_simd_v<_Simd_>)
{
	return _Ternarylogic<_Simd_::__isa, _Simd_::__width>()(
		__data(__x), __data(__y), __data(__z), __imm8);
}

template <
	non_memory_where_expression_type	_WhereExpression_,
	simd_type							_Simd_,
	uint8								_TernaryMask_>
raze_nodiscard raze_always_inline _Simd_ ternarylogic(
	const _WhereExpression_&						__where_x,
	const _Simd_&									__y,
	const _Simd_&									__z,
	std::integral_constant<uint8, _TernaryMask_>	__imm8) noexcept
		requires((_TernaryMask_ >= 0 && _TernaryMask_ <= 255) && 
			__is_valid_simd_v<_Simd_>)
{
	return _WhereExpression_::__ternarylogic(__where_x, __y, __z, __imm8);
}

template <
	simd_type							_Simd_,
	non_memory_where_expression_type	_WhereExpression_,
	uint8								_TernaryMask_>
raze_nodiscard raze_always_inline _Simd_ ternarylogic(
	const _Simd_&									__x,
	const _WhereExpression_&						__where_y,
	const _Simd_&									__z,
	std::integral_constant<uint8, _TernaryMask_>	__imm8) noexcept
		requires((_TernaryMask_ >= 0 && _TernaryMask_ <= 255) && 
			__is_valid_simd_v<_Simd_>)
{
	return _WhereExpression_::__ternarylogic(__x, __where_y, __z, __imm8);
}

template <
	simd_type							_Simd_,
	non_memory_where_expression_type	_WhereExpression_,
	uint8								_TernaryMask_>
raze_nodiscard raze_always_inline _Simd_ ternarylogic(
	const _Simd_&									__x,
	const _Simd_&									__y,
	const _WhereExpression_&						__where_z,
	std::integral_constant<uint8, _TernaryMask_>	__imm8) noexcept
		requires((_TernaryMask_ >= 0 && _TernaryMask_ <= 255) && 
			__is_valid_simd_v<_Simd_>)
{
	return _WhereExpression_::__ternarylogic(__x, __y, __where_z, __imm8);
}


/**
 * @brief Generates a compile‑time ternary‑logic mask from a lazy expression.
 *
 * This function evaluates a ternary boolean expression represented by
 * @_Expression_ and converts it into an 8‑bit truth‑table mask. 
 *
 * The resulting mask is returned as a `std::integral_constant<uint8, Mask>`,
 * making it suitable for direct use in `ternarylogic(...)` or other APIs that
 * require a compile‑time ternary mask.
 *
 * @tparam _Expression_
 *     A lazy ternary‑logic expression type satisfying `ternary_mask_expression_type`.
 *
 * @return
 *     A `std::integral_constant<uint8, Mask>` representing the truth table of
 *     @_Expression_.
 * 
 * @example
 *     auto r = ternarylogic(x, y, z, as_ternary_mask<A ^ B ^ C>());
*/
template <ternary_mask_expression_type _Expression_>
raze_nodiscard consteval auto as_ternary_mask() noexcept {
	return std::integral_constant<uint8, __as_ternary_mask<_Expression_>()>{};
}

__RAZE_VX_NAMESPACE_END
