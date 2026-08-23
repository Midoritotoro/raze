#pragma once 

#include <raze/algorithm/find/AnyOf.h>
#include <src/raze/algorithm/EqualTo.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Contains : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_, class _Value_, class Projection = std::identity>
	raze_nodiscard constexpr raze_always_inline bool operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, Projection __proj = {}) const noexcept
	{
		return algorithm::any_of[_Traits_::traits()](std::move(__first), std::move(__last),
			algorithm::equal_to(__v), traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, class _Value_, class Projection = std::identity>
	constexpr raze_always_inline bool operator()(Range&& __r, 
		const _Value_& __v, Projection __proj = {}) const noexcept
	{
		return algorithm::any_of[_Traits_::traits()](std::forward<Range>(__r),
			algorithm::equal_to(__v), traits::fwd_fn(__proj));
	}
};

constexpr inline auto contains = raze::options::function_with_traits<_Contains>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
