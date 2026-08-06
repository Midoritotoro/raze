#pragma once 

#include <raze/algorithm/find/Search.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Contains_subrange : _Traits_ {
	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<_Iterator1_, _Sentinel1_, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __end = __last1;
		return algorithm::search[_Traits_::traits()](std::move(__first1), std::move(__last1),
			std::move(__first2), std::move(__last2), traits::__fwd_fn(__pred), 
			traits::__fwd_fn(__proj1), traits::__fwd_fn(__proj2)) != __end;
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __r1, _Range2_&& __r2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
				std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __end = std::ranges::end(__r1);
		return algorithm::search[_Traits_::traits()](std::forward<_Range1_>(__r1), std::forward<_Range2_>(__r2),
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj1), traits::__fwd_fn(__proj2)) != __end;
	}
};

constexpr inline auto contains_subrange = raze::options::function_with_traits<_Contains_subrange>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
