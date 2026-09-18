#pragma once 

#include <raze/algorithm/replace/ReplaceIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct replace_t : Traits {
	template <std::permutable Iter, std::sentinel_for<Iter> Sent,
		class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline void operator()(Iter first, Sent last,
		const T1& old_value, const T2& new_value, Proj proj = {}) const
	{
		replace_if[Traits::traits()](std::move(first), std::move(last), algorithm::equal_to(
			function_return_type<Proj, std::iter_value_t<Iter>>(old_value)),
			new_value, traits::fwd_fn(proj));
	}

	template <std::ranges::input_range Range, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline void operator()(Range&& r, const T1& old_value,
		const T2& new_value, Proj proj = {}) const
			requires(std::permutable<std::ranges::iterator_t<Range>>)
	{
		replace_if[Traits::traits()](std::forward<Range>(r), algorithm::equal_to(
			function_return_type<Proj, std::ranges::range_value_t<Range>>(old_value)),
			new_value, traits::fwd_fn(proj));
	}
};

constexpr inline auto replace = options::function_with_traits<replace_t>[options::unroll<4>][replace_strategy];

__RAZE_ALGORITHM_NAMESPACE_END