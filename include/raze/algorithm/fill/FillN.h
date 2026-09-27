#pragma once

#include <raze/algorithm/fill/Fill.h>
#include <iterator>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct fill_n_t : Traits {
	template <std::input_or_output_iterator OutIt, class Size = std::iter_difference_t<OutIt>,
		class T = std::iter_value_t<OutIt>>
	constexpr raze_always_inline OutIt operator()(OutIt first, Size n,
		const std::type_identity_t<T>& value) const requires(std::output_iterator<OutIt, const T&>)
	{
		if (n <= 0) return first;
		return fill[Traits::traits()](std::counted_iterator<OutIt>(
			std::move(first), n), std::default_sentinel, value).base();
	}
};

constexpr inline auto fill_n = options::function_with_traits<fill_n_t>[options::unroll<4>][fill_strategy];

__RAZE_ALGORITHM_NAMESPACE_END