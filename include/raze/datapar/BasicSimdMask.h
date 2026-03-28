#pragma once 

#include <src/raze/datapar/bitwise/MaskBits.h>
#include <src/raze/datapar/bitwise/MaskElementReference.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <bitset>


__RAZE_DATAPAR_NAMESPACE_BEGIN

template <
	class _FirstMaskType_,
	class _SecondMaskType_>
concept __compatible_mask =
	(sizeof(typename _FirstMaskType_::element_type) == sizeof(typename _SecondMaskType_::element_type)) &&
	(_FirstMaskType_::__isa == _SecondMaskType_::__isa) &&
	(_FirstMaskType_::__width == _SecondMaskType_::__width);

template <
	class _Type_,
	class _Abi_>
class simd_mask
{
	static_assert(type_traits::__is_generation_supported_v<_Abi_::isa>);
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);

	using _Impl = _Mask_implementation<_Type_, _Abi_>;
	using __bits_type = _Mask_bits<_Type_, _Abi_>;
public:
	using mask_type		= typename _Impl::mask_type;
	using element_type	= _Type_;
	using abi_type		= _Abi_;

	static constexpr auto __width = _Abi_::width;
	static constexpr auto __isa = _Abi_::isa;

	simd_mask() = default;
	simd_mask(const simd_mask& __mask) = default;
	simd_mask(simd_mask&& __mask) = default;
	~simd_mask() = default;

	simd_mask(bool __value) noexcept {
		_mask = _Impl::__broadcast(__value);
	}

	template <class _Simd_>
	simd_mask(_Simd_ __mask) noexcept requires(std::is_convertible_v<_Simd_, mask_type>):
		_mask(static_cast<mask_type>(__mask))
	{}

	template <uint64 _Bits_>
	simd_mask(const std::bitset<_Bits_>& __bitset) noexcept:
		_mask(__from_bitset(__bitset)) 
	{}
	
	template <
		class _ForwardIterator_,
		class _AlignmentPolicy_ = __unaligned_policy>
	simd_mask(
		const _ForwardIterator_	__first,
		_AlignmentPolicy_&&		__alignment_policy = {}) noexcept
			requires(type_traits::is_iterator_v<_ForwardIterator_>
				&& type_traits::is_iterator_forward_ranges_v<_ForwardIterator_>)
	{
		copy_from(__first, __alignment_policy);
	}

	raze_nodiscard raze_always_inline __bits_type bits() const noexcept {
		return __bits_type(_mask);
	}

	template <
		class _ForwardIterator_, 
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_always_inline void copy_from(
		const _ForwardIterator_ __first,
		_AlignmentPolicy_&&		__alignment_policy = {}) noexcept
			requires(std::convertible_to<type_traits::iterator_value_type<_ForwardIterator_>, bool>)
	{
		__copy_from_unchecked(algorithm::__unwrap_iterator(__first), __alignment_policy);
	}

	template <
		class _OutputIterator_,
		class _AlignmentPolicy_ = __unaligned_policy>
	raze_always_inline void copy_to(
		_OutputIterator_	__first,
		_AlignmentPolicy_&& __alignment_policy = {}) noexcept
	{
		__copy_to_unchecked(algorithm::__unwrap_iterator(__first), __alignment_policy);
	}

	raze_always_inline bool operator[](int32 __index) const noexcept {
		return __mask_element_reference<_Type_, _Abi_>(_mask, __index);
	}

	raze_always_inline __mask_element_reference<_Type_, _Abi_> operator[](int32 __index) noexcept {
		return __mask_element_reference<_Type_, _Abi_>(_mask, __index);
	}
	
	template <uint32 _Elements_>
	friend raze_always_inline simd_mask operator>>(
		const simd_mask&							__left,
		std::integral_constant<uint32, _Elements_>	__elements) noexcept
	{
		return _Impl::__slide_right(__left, __elements);
	}

	friend raze_always_inline simd_mask operator>>(
		const simd_mask&	__left,
		uint32				__elements) noexcept
	{
		return _Impl::__slide_right(__left, __elements);
	}

	template <uint32 _Elements_>
	friend raze_always_inline simd_mask operator<<(
		const simd_mask&							__left,
		std::integral_constant<uint32, _Elements_>	__elements) noexcept
	{
		return _Impl::__slide_left(__left, __elements);
	}

	friend raze_always_inline simd_mask operator<<(
		const simd_mask&	__left,
		uint32				__elements) noexcept
	{
		return _Impl::__slide_left(__left, __elements);
	}

	template <uint32 _Elements_>
	raze_always_inline simd_mask& operator<<=(std::integral_constant<uint32, _Elements_> __elements) noexcept {
		return *this = (*this << __elements);
	}

	raze_always_inline simd_mask& operator<<=(uint32 __elements) noexcept {
		return *this = (*this << __elements);
	}

	template <uint32 _Elements_>
	raze_always_inline simd_mask& operator>>=(std::integral_constant<uint32, _Elements_> __elements) noexcept {
		return *this = (*this >> __elements);
	}

	raze_always_inline simd_mask& operator>>=(uint32 __elements) noexcept {
		return *this = (*this >> __elements);
	}

	raze_always_inline simd_mask& operator=(const simd_mask& __other) noexcept {
		_mask = __other._mask;
		return *this;
	}

	template <class _OtherMask_>
	friend raze_always_inline simd_mask operator&(
		const simd_mask&	__left,
		const _OtherMask_&	__right) noexcept 
			requires(__compatible_mask<simd_mask, _OtherMask_>)
	{
		return _Impl::__bit_and(__left, __right);
	}

	template <class _OtherMask_>
	friend raze_always_inline simd_mask operator|(
		const simd_mask&	__left,
		const _OtherMask_&	__right) noexcept 
			requires(__compatible_mask<simd_mask, _OtherMask_>)
	{
		return _Impl::__bit_or(__left, __right);
	}

	template <class _OtherMask_>
	friend raze_always_inline simd_mask operator^(
		const simd_mask&	__left,
		const _OtherMask_&	__right) noexcept
			requires(__compatible_mask<simd_mask, _OtherMask_>)
	{
		return _Impl::__bit_xor(__left, __right);
	}

	template <class _OtherMask_>
	raze_always_inline simd_mask& operator&=(const _OtherMask_& __other) noexcept 
		requires(__compatible_mask<simd_mask, _OtherMask_>)
	{
		return *this = (*this & __other);
	}

	template <class _OtherMask_>
	raze_always_inline simd_mask& operator|=(const _OtherMask_& __other) noexcept 
		requires(__compatible_mask<simd_mask, _OtherMask_>)
	{
		return *this = (*this | __other);
	}

	template <class _OtherMask_>
	raze_always_inline simd_mask& operator^=(const _OtherMask_& __other) noexcept 
		requires(__compatible_mask<simd_mask, _OtherMask_>)
	{
		return *this = (*this ^ __other);
	}

	raze_always_inline simd_mask operator!() const noexcept {
		return _Impl::__bit_not(_mask);
	}

	friend raze_always_inline simd_mask operator!=(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_xor(__left, __right);
	}

	friend raze_always_inline simd_mask operator==(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return !(__left != __right);
	}

	raze_nodiscard static constexpr raze_always_inline int32 bit_width() noexcept {
		return _Impl::__bit_width();
	}

	raze_nodiscard static constexpr raze_always_inline int32 elements() noexcept {
		return _Impl::__elements();
	}

	raze_nodiscard raze_always_inline bool __all_of() const noexcept {
		return _Impl::__all_of(_mask);
	}

	raze_nodiscard raze_always_inline bool __any_of() const noexcept {
		return _Impl::__any_of(_mask);
	}

	raze_nodiscard raze_always_inline bool __none_of() const noexcept {
		return _Impl::__none_of(_mask);
	}

	raze_nodiscard raze_always_inline bool __some_of() const noexcept {
		return _Impl::__some_of(_mask);
	}

	raze_always_inline operator mask_type() const noexcept {
		return _mask;
	}

	raze_nodiscard raze_always_inline int32 __count_set() const noexcept {
		return _Impl::__count_set(_mask);
	}

	raze_nodiscard raze_always_inline int32 
		__count_trailing_zero_bits() const noexcept
	{
		return _Impl::__count_trailing_zero_bits(_mask);
	}

	raze_nodiscard raze_always_inline int32
		__count_leading_zero_bits() const noexcept
	{
		return _Impl::__count_leading_zero_bits(_mask);
	}

	raze_nodiscard raze_always_inline int32 
		__count_trailing_one_bits() const noexcept 
	{
		return _Impl::__count_trailing_one_bits(_mask);
	}

	raze_nodiscard raze_always_inline int32 
		__count_leading_one_bits() const noexcept 
	{
		return _Impl::__count_leading_one_bits(_mask);
	}

	template <sizetype _Bits_>
	raze_nodiscard raze_always_inline static mask_type
		__from_bitset(const std::bitset<_Bits_>& __bitset) noexcept 
	{
		return _Impl::__from_bitset(__bitset);
	}

	template <
		class _UnwrappedForwardIterator_, 
		class _AlignmentPolicy_>
	raze_always_inline void __copy_from_unchecked(
		_UnwrappedForwardIterator_	__first,
		_AlignmentPolicy_&&			__alignment_policy) noexcept 
	{
		_mask = _Impl::__copy_from_unchecked(__first, __alignment_policy);
	}

	template <
		class _UnwrappedOutputIterator_, 
		class _AlignmentPolicy_>
	raze_always_inline void __copy_to_unchecked(
		_UnwrappedOutputIterator_	__first,
		_AlignmentPolicy_&&			__alignment_policy) noexcept
	{
		_Impl::__copy_to_unchecked(__first, _mask, __alignment_policy);
	}

	raze_always_inline void __clear_left() noexcept {
		_mask = _Impl::__clear_left(_mask);
	}

	raze_always_inline void __clear_right() noexcept {
		_mask = _Impl::__clear_right(_mask);
	}

	raze_nodiscard raze_always_inline bool __is_contiguous(uint32 __n, uint32 __k) const noexcept {
		return _Impl::__is_contiguous(_mask, __n, __k);
	}
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END