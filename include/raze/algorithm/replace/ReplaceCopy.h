#pragma once 

#include <raze/algorithm/replace/ReplaceCopyIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct replace_copy_t : Traits {
	template <std::input_iterator InIt, std::sentinel_for<InIt> Sent,
		std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<InIt, OutIt> operator()(
		InIt first, Sent sent, OutIt out, const T1& old, const T2& v, Proj proj = {}) const
			requires(std::indirectly_copyable<InIt, OutIt> && std::indirectly_writable<OutIt, const T2&>)
	{
		return replace_copy_if[Traits::traits()](std::move(first), std::move(sent), std::move(out),
			algorithm::equal_to(function_return_type<Proj, std::iter_value_t<InIt>>(old)),
			v, traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, std::weakly_incrementable OutIt, class T1, class T2, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<R>, OutIt> operator()(
		R&& r, OutIt out, const T1& old, const T2& v, Proj proj = {}) const
			requires(std::indirectly_copyable<std::ranges::iterator_t<R>, OutIt> && std::indirectly_writable<OutIt, const T2&>)
	{
		return replace_copy_if[Traits::traits()](std::forward<R>(r), std::move(out),
			algorithm::equal_to(function_return_type<Proj, std::ranges::range_value_t<R>>(old)),
			v, traits::fwd_fn(proj));
	}
};

constexpr inline auto replace_copy = options::function_with_traits<replace_copy_t>[options::unroll<4>][replace_copy_strategy];

__RAZE_ALGORITHM_NAMESPACE_END
