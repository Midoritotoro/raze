#pragma once

#include <raze/algorithm/remove/RemoveIf.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct remove_t : Traits {
	template <std::permutable It, std::sentinel_for<It> Sent, class T = std::iter_value_t<It>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::subrange<It> operator()(
		It first, Sent last, const std::type_identity_t<T>& value, Proj proj = {}) const
	{
		return remove_if[Traits::traits()](std::move(first), std::move(last),
			algorithm::equal_to(value), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range R, class T = std::ranges::range_value_t<R>, class Proj = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<R> operator()(
		R&& r, const std::type_identity_t<T>& value, Proj proj = {}) const
			requires(std::permutable<std::ranges::iterator_t<R>>)
	{
		return remove_if[Traits::traits()](std::forward<R>(r),
			algorithm::equal_to(value), traits::fwd_fn(proj));
	}
};

constexpr inline auto remove = options::function_with_traits<remove_t>[remove_strategy];

__RAZE_ALGORITHM_NAMESPACE_END