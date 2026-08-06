#pragma once 

#include <raze/algorithm/find/Mismatch.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Starts_with : _Traits_ {
	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Iterator1_ __first1,
		_Sentinel1_ __sent1, _Iterator2_ __first2, _Sentinel2_ __sent2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires (std::forward_iterator<_Iterator1_> || std::sized_sentinel_for<_Sentinel1_, _Iterator1_>) &&
                 (std::forward_iterator<_Iterator2_> || std::sized_sentinel_for<_Sentinel2_, _Iterator2_>) &&
                 std::indirectly_comparable<_Iterator1_, _Iterator2_, _Predicate_, _Projection1_, _Projection2_>
	{
		return algorithm::mismatch[_Traits_::traits()](std::move(__first1), std::move(__sent1), 
			std::move(__first2), __sent2, traits::__fwd_fn(__pred), traits::__fwd_fn(__proj1), 
			traits::__fwd_fn(__proj2)).in2 == __sent2;
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __r1, _Range2_&& __r2, 
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires (std::ranges::forward_range<_Range1_> || std::ranges::sized_range<_Range1_>) &&
				(std::ranges::forward_range<_Range2_> || std::ranges::sized_range<_Range2_>) &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,  
				std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>
	{
		auto __end = std::ranges::end(__r2);
		return algorithm::mismatch[_Traits_::traits()](std::forward<_Range1_>(__r1),
			std::forward<_Range2_>(__r2), traits::__fwd_fn(__pred), traits::__fwd_fn(__proj1),
			traits::__fwd_fn(__proj2)).in2 == __r2;
	}
};

constexpr inline auto starts_with = raze::options::function_with_traits<_Starts_with>;

__RAZE_ALGORITHM_NAMESPACE_END