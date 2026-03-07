#pragma once 

#include <src/raze/datapar/bitwise/MaskImplementation.h>
#include <src/raze/datapar/bitwise/MaskElementReference.h>
#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <bitset>



__RAZE_DATAPAR_NAMESPACE_BEGIN


template <
	arch::ISA	_ISA_,
	typename	_Type_,
	uint32		_SimdWidth_>
class simd_mask
{
	static_assert(type_traits::__is_generation_supported_v<_ISA_>);
	static_assert(type_traits::__is_vector_type_supported_v<_Type_>);

	using _Impl = _Mask_implementation<_ISA_, _Type_, _SimdWidth_>;
public:
	using mask_type = typename _Impl::mask_type;
	using element_type = _Type_;

	static constexpr auto __width = _SimdWidth_;
	static constexpr auto __isa = _ISA_;

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

	constexpr raze_always_inline bool operator[](int32 __index) const noexcept {
		return __mask_element_reference<_ISA_, _Type_, _SimdWidth_>(_mask, __index);
	}

	constexpr raze_always_inline __mask_element_reference<_ISA_, 
		_Type_, _SimdWidth_> operator[](int32 __index) noexcept 
	{
		return __mask_element_reference<_ISA_, _Type_, _SimdWidth_>(_mask, __index);
	}

	friend constexpr raze_always_inline simd_mask operator&(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_and(__left, __right);
	}

	friend constexpr raze_always_inline simd_mask operator|(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_or(__left, __right);
	}

	friend constexpr raze_always_inline simd_mask operator^(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_xor(__left, __right);
	}

	constexpr raze_always_inline simd_mask& 
		operator&=(const simd_mask& __other) noexcept 
	{
		return *this & __other;
	}

	constexpr raze_always_inline simd_mask&
		operator|=(const simd_mask& __other) noexcept
	{
		return *this | __other;
	}

	constexpr raze_always_inline simd_mask& 
		operator^=(const simd_mask& __other) noexcept 
	{
		return *this ^ __other;
	}

	constexpr raze_always_inline simd_mask operator!() const noexcept {
		return _Impl::__bit_not(_mask);
	}

	friend constexpr raze_always_inline simd_mask operator!=(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return _Impl::__bit_xor(__left, __right);
	}

	friend constexpr raze_always_inline simd_mask operator==(
		const simd_mask& __left,
		const simd_mask& __right) noexcept
	{
		return __left & __right;
	}

	static constexpr raze_always_inline int32 bit_width() noexcept {
		return _Impl::__bit_width();
	}

	static constexpr raze_always_inline int32 elements() noexcept {
		return _Impl::__elements();
	}

	raze_nodiscard raze_always_inline constexpr bool __all_of() const noexcept {
		return _Impl::__all_of(_mask);
	}

	raze_nodiscard raze_always_inline constexpr bool __any_of() const noexcept {
		return _Impl::__any_of(_mask);
	}

	raze_nodiscard raze_always_inline constexpr bool __none_of() const noexcept {
		return _Impl::__none_of(_mask);
	}

	raze_nodiscard raze_always_inline constexpr bool __some_of() const noexcept {
		return _Impl::__some_of(_mask);
	}

	raze_always_inline constexpr operator mask_type() const noexcept {
		return _mask;
	}

	raze_nodiscard raze_always_inline constexpr int32 __count_set() const noexcept {
		return _Impl::__count_set(_mask);
	}

	raze_nodiscard raze_always_inline constexpr int32 
		__count_trailing_zero_bits() const noexcept
	{
		return _Impl::__count_trailing_zero_bits(_mask);
	}

	raze_nodiscard raze_always_inline constexpr int32
		__count_leading_zero_bits() const noexcept
	{
		return _Impl::__count_leading_zero_bits(_mask);
	}

	raze_nodiscard raze_always_inline constexpr int32 
		__count_trailing_one_bits() const noexcept 
	{
		return _Impl::__count_trailing_one_bits(_mask);
	}

	raze_nodiscard raze_always_inline constexpr int32 
		__count_leading_one_bits() const noexcept 
	{
		return _Impl::__count_leading_one_bits(_mask);
	}

	template <sizetype _Bits_>
	raze_nodiscard raze_always_inline static constexpr mask_type
		__from_bitset(const std::bitset<_Bits_>& __bitset) noexcept 
	{
		return _Impl::__from_bitset(__bitset);
	}

	template <
		class _UnwrappedForwardIterator_, 
		class _AlignmentPolicy_>
	raze_nodiscard raze_always_inline constexpr void 
		__copy_from_unchecked(
			const _UnwrappedForwardIterator_	__first,
			_AlignmentPolicy_&&					__alignment_policy) noexcept 
	{
		_mask = _Impl::__copy_from_unchecked(__first, __alignment_policy);
	}

	template <
		class _UnwrappedOutputIterator_, 
		class _AlignmentPolicy_>
	raze_nodiscard raze_always_inline constexpr void 
		__copy_to_unchecked(
			_UnwrappedOutputIterator_	__first,
			_AlignmentPolicy_&&			__alignment_policy) noexcept
	{
		_Impl::__copy_to_unchecked(__first, __alignment_policy);
	}
private:
	mask_type _mask = 0;
};

__RAZE_DATAPAR_NAMESPACE_END