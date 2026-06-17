#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Any_of : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
	{
		return algorithm::find_if(std::move(__first), __last,
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj)) != __last;
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
	{
		auto __last = std::ranges::end(__range);
		return algorithm::find_if(std::move(__range), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj)) != __last;
	}
};

constexpr inline auto any_of = raze::options::function_with_traits<_Any_of>;


__RAZE_ALGORITHM_NAMESPACE_END
