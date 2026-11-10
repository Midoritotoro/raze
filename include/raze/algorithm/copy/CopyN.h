#pragma once

#include <raze/algorithm/copy/Copy.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct copy_n_t : Traits {
	template <std::input_iterator InIt,  std::weakly_incrementable OutIt>
		constexpr raze_always_inline std::ranges::in_out_result<InIt, OutIt> operator()(InIt first,
			std::iter_difference_t<InIt> n, OutIt out) const requires(std::indirectly_copyable<InIt, OutIt>)
	{	
		auto [in_last, out_last] = copy[Traits::traits()](std::counted_iterator<InIt>(
			std::move(first), n), std::default_sentinel, std::move(out));
		return { std::move(in_last).base(), std::move(out_last) };
	}
};

constexpr inline auto copy_n = options::function_with_traits<copy_n_t>[options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END
