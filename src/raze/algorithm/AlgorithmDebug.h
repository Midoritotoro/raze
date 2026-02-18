#pragma once 

#include <src/raze/utility/Assert.h>
#include <raze/compatibility/Inline.h>

#include <src/raze/type_traits/IteratorCheck.h>
#include <src/raze/type_traits/IntegralProperties.h>

#include <raze/math/IntegralTypesConversions.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Type_>
raze_constexpr_cxx20 void __verify_range__(
	const _Type_* const __first,
	const _Type_* const __last) noexcept
{
	raze_debug_assert_log(__first <= __last, "transposed pointer range");
}

template <
	class _Iterator_,
	class _Sentinel_>
raze_constexpr_cxx20 void __verify_range__(
	const _Iterator_& __first,
	const _Sentinel_& __last) noexcept
{
#if !defined(NDEBUG)
	if constexpr (std::is_pointer_v<_Iterator_> && std::is_pointer_v<_Sentinel_>) {
		raze_debug_assert_log(__first <= __last, "transposed pointer range");
		return;
	}
	else if constexpr (type_traits::__is_range_verifiable_v<_Iterator_, _Sentinel_>) {
		__verify_range__(
			const_cast<const char*>(reinterpret_cast<const volatile char*>(std::to_address(__first))),
			const_cast<const char*>(reinterpret_cast<const volatile char*>(std::to_address(__last))));
	}
#endif
}

#if !defined(__verify_range)
#  if !defined(NDEBUG)
#    define __verify_range(__first, __last)  __verify_range__(__first, __last) 
#  else
#    define __verify_range(__first, __last) 
#  endif // !defined(NDEBUG)
#endif // !defined(__verifyRange)

__RAZE_ALGORITHM_NAMESPACE_END
