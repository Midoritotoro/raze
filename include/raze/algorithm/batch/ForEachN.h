#pragma once

#include <raze/algorithm/batch/ForEach.h>
#include <src/raze/algorithm/MakeCounted.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct for_each_n_t : Traits {
	template <std::input_iterator InIt, class F, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::in_fun_result<InIt, F> operator()(
		InIt first, std::iter_difference_t<InIt> n, F f, Proj proj = {}) const
			requires(std::indirectly_unary_invocable<F, std::projected<InIt, Proj>>)
	{
		auto r = for_each[Traits::traits()](make_counted(std::move(first), n),
			std::default_sentinel, traits::fwd_fn(f), traits::fwd_fn(proj));
		return { r.in.base(), std::move(r.fun)};
	}
};

constexpr inline auto for_each_n = options::function_with_traits<for_each_n_t>[options::unroll<4>][for_each_strategy];

__RAZE_ALGORITHM_NAMESPACE_END