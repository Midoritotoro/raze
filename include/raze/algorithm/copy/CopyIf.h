#pragma once 

#include <raze/algorithm/remove/RemoveCopy.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Copy_if : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		std::weakly_incrementable _Output_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::pair<_Iterator_, _Output_> operator()(_Iterator_ __first, _Sentinel_ __last,
		_Output_ __output, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>> &&
			std::indirectly_copyable<_Iterator_, _Output_>)
	{
		return raze::algorithm::remove_copy_if[_Traits_::traits()](std::move(__first), std::move(__last),
			std::move(__output), algorithm::make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _Output_,
		class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::pair<std::ranges::iterator_t<_Range_>, _Output_> operator()(
		_Range_&& __range, _Output_ __output, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>> 
			&& std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _Output_>)
	{
		return raze::algorithm::remove_copy_if[_Traits_::traits()](std::forward<_Range_>(__range),
			std::move(__output), algorithm::make_not_fn(__pred), type_traits::__pass_function(__proj));
	}
};

constexpr inline auto copy_if = raze::options::function_with_traits<_Copy_if>;

__RAZE_ALGORITHM_NAMESPACE_END