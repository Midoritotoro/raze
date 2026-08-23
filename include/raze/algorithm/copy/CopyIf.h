#pragma once 

#include <raze/algorithm/remove/RemoveCopy.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Copy_if : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		std::weakly_incrementable _Output_, class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::pair<_Iterator_, _Output_> operator()(_Iterator_ __first, _Sentinel_ __last,
		_Output_ __output, Predicate __pred, Projection __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<Predicate, std::projected<_Iterator_, Projection>> &&
			std::indirectly_copyable<_Iterator_, _Output_>)
	{
		return raze::algorithm::remove_copy_if[_Traits_::traits()](std::move(__first), std::move(__last),
			std::move(__output), algorithm::make_not_fn(__pred), traits::fwd_fn(__proj));
	}

	template <std::ranges::input_range Range, std::weakly_incrementable _Output_,
		class Predicate, class Projection = std::identity>
	constexpr raze_always_inline std::pair<std::ranges::iterator_t<Range>, _Output_> operator()(
		Range&& __range, _Output_ __output, Predicate __pred, Projection __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<Predicate, std::projected<std::ranges::iterator_t<Range>, Projection>> 
			&& std::indirectly_copyable<std::ranges::iterator_t<Range>, _Output_>)
	{
		return raze::algorithm::remove_copy_if[_Traits_::traits()](std::forward<Range>(__range),
			std::move(__output), algorithm::make_not_fn(__pred), traits::fwd_fn(__proj));
	}
};

constexpr inline auto copy_if = raze::options::function_with_traits<_Copy_if>;

__RAZE_ALGORITHM_NAMESPACE_END