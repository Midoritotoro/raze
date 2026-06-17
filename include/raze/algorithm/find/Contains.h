#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Contains : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_ = std::iter_value_t<_Iterator_>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		return algorithm::find(std::move(__first), __last, __v,
			type_traits::__pass_function(__proj)) != __last;
	}

	template <std::ranges::input_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>,
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::iterator_t<_Range_> operator()(
		_Range_&& __range, const std::type_identity_t<_Value_>& __v, _Projection_ __proj = {}) const noexcept
	{
		auto __last = std::ranges::end(__range);
		return algorithm::find_if(std::move(__range), __v,
			type_traits::__pass_function(__proj)) != __last;
	}
};

constexpr inline auto contains = raze::options::function_with_traits<_Contains>;


__RAZE_ALGORITHM_NAMESPACE_END
