#pragma once 

#include <raze/Types.h>

#include <src/raze/algorithm/MsvcIteratorUnwrap.h>
#include <src/raze/algorithm/AdvanceBytes.h>


__RAZE_MEMORY_NAMESPACE_BEGIN

template <
	typename _FirstForwardIterator_,
	typename _SecondForwardIterator_>
raze_nodiscard raze_always_inline bool intersects(
	const _FirstForwardIterator_	__first_begin,
	const _FirstForwardIterator_	__first_end,
	const _SecondForwardIterator_	__second_begin) noexcept
{
	const auto __first_range_begin_address	= reinterpret_cast<const volatile char*>(std::to_address(__first_begin));
	const auto __second_range_begin_address	= reinterpret_cast<const volatile char*>(std::to_address(__second_begin));

	const auto __first_range_end_address		= reinterpret_cast<const volatile char*>(std::to_address(__first_end));
	const auto __second_range_end_address	= reinterpret_cast<const volatile char*>(__second_range_begin_address) 
		+ algorithm::__byte_length(__first_range_begin_address, __first_range_end_address);

	return ((__first_range_begin_address > __second_range_begin_address) && (__first_range_begin_address < __second_range_end_address)) ||
		((__second_range_begin_address > __first_range_begin_address) && (__second_range_begin_address < __first_range_end_address));
}

template <
	typename _FirstForwardIterator_,
	typename _SecondForwardIterator_>
void __check_intersection(
	const _FirstForwardIterator_	__first_begin,
	const _FirstForwardIterator_	__first_end,
	const _SecondForwardIterator_	__second_begin) noexcept
{
#if !defined(NDEBUG)
	using _FirstForwardIteratorUnwrappedType_	= algorithm::__unwrapped_iterator_type<_FirstForwardIterator_>;
	using _SecondForwardIteratorUnwrappedType_	= algorithm::__unwrapped_iterator_type<_SecondForwardIterator_>;

	if constexpr (
		type_traits::is_iterator_contiguous_v<_FirstForwardIteratorUnwrappedType_> &&
		type_traits::is_iterator_contiguous_v<_SecondForwardIteratorUnwrappedType_>)
	{
		raze_debug_assert(memory::intersects(std::move(__first_begin),
			std::move(__first_end), std::move(__second_begin)) == false);
	}
#endif
}

__RAZE_MEMORY_NAMESPACE_END
