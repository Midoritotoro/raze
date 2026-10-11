#pragma once

#include <raze/algorithm/remove/RemoveCopyIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct remove_copy_t : Traits {
	template <std::permutable It, std::sentinel_for<It> Sent, class OutIt, 
		class T = std::iter_value_t<It>, class Proj = std::identity>
	constexpr raze_always_inline decltype(auto) operator()(It first, Sent last, 
		OutIt out, const std::type_identity_t<T>& v, Proj proj = {}) const
	{
		return remove_copy_if[Traits::traits()](std::move(first), std::move(last), 
			std::move(out), algorithm::equal_to(v), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class OutIt, class T = std::ranges::range_value_t<R>, class Proj = std::identity>
	constexpr raze_always_inline decltype(auto) operator()(R&& r, OutIt out,
		const std::type_identity_t<T>& v, Proj proj = {}) const
	{
		return remove_copy_if[Traits::traits()](std::forward<R>(r), std::move(out),
			algorithm::equal_to(v), traits::fwd_fn(proj));
	}
};

constexpr inline auto remove_copy = options::function_with_traits<remove_copy_t>[remove_copy_strategy];

__RAZE_ALGORITHM_NAMESPACE_END