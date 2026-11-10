#pragma once 

#include <raze/algorithm/remove/RemoveCopyIf.h>
#include <src/raze/algorithm/NotFn.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class Traits>
struct copy_if_t : Traits {
	template <std::input_iterator Iterator, std::sentinel_for<Iterator> Sentinel,
		std::weakly_incrementable Output, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline auto operator()(Iterator first, Sentinel last,
		Output output, Predicate pred, Projection proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<Iterator, Projection>> &&
			std::indirectly_copyable<Iterator, Output>)
	{
		return raze::algorithm::remove_copy_if[Traits::traits()](std::move(first), std::move(last),
			std::move(output), algorithm::make_not_fn(pred), traits::fwd_fn(proj));
	}

	template <std::ranges::input_range Range, std::weakly_incrementable Output,
		class Predicate, class Projection = std::identity>
	constexpr raze_always_inline auto operator()(Range&& range, Output output, Predicate pred, Projection proj = {}) const
		requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> 
			&& std::indirectly_copyable<std::ranges::iterator_t<Range>, Output>)
	{
		return raze::algorithm::remove_copy_if[Traits::traits()](std::forward<Range>(range),
			std::move(output), algorithm::make_not_fn(pred), traits::fwd_fn(proj));
	}
};

constexpr inline auto copy_if = raze::options::function_with_traits<copy_if_t>;

__RAZE_ALGORITHM_NAMESPACE_END