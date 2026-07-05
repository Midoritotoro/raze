#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Ends_with : _Traits_ {
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	struct __impl {
		_Iterator1_ _iterator1;
		_Iterator2_ _iterator2;
		_Sentinel1_ _sentinel1;
		_Sentinel2_ _sentinel2;
		_Predicate_ _predicate;
		_Projection1_ _proj1;
		_Projection2_ _proj2;
		bool _result;

		constexpr explicit __impl(_Iterator1_ __it1, _Sentinel1_ __sent1, _Iterator2_ __it2,
			_Sentinel2_ __sent2, _Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) noexcept:
				_iterator1(__it1), _sentinel1(__sent1), _iterator2(__it2), _sentinel2(__sent2),
				_predicate(__pred), _proj1(__proj1), _proj2(__proj2)
		{}

		template <class _Tag_>
		constexpr raze_always_inline bool operator()(_Tag_) noexcept {
			if (_iterator1 != _sentinel1 && _iterator2 != _sentinel2) {
				if (!_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
					_result = false;
					return true;
				}

				++_iterator1;
				++_iterator2;

				return false;
			}

			_result = _iterator2 == _sentinel2;
			return true;
		}

		raze_always_inline constexpr bool result() const noexcept {
			return _result;
		}
	};

	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::ranges::equal_to, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<_Iterator1_, _Sentinel1_, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __ends_with_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_sentinel<_Iterator1_>(std::move(__last1)),
			type_traits::__ranges_unwrap_iterator<_Sentinel2_>(std::move(__first2)), 
			type_traits::__ranges_unwrap_sentinel<_Iterator2_>(std::move(__last2)), 
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::ranges::equal_to, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __range1, _Range2_&& __range2, 
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires((!constexpr_sized_range<_Range1_> || !constexpr_sized_range<_Range2_>) &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __ends_with_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::ranges::begin(__range1)),
			type_traits::__unchecked_end(__range1), type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::ranges::begin(__range2)), 
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::ranges::equal_to, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __range1, _Range2_&& __range2, 
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(constexpr_sized_range<_Range1_> && constexpr_sized_range<_Range2_> &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		if constexpr (__range_constexpr_size<_Range1_>() < __range_constexpr_size<_Range2_>()) return false;
		else return __ends_with_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::ranges::begin(__range1)),
			type_traits::__unchecked_end(__range1), type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::ranges::begin(__range2)),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));
	}
private:
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	constexpr raze_always_inline bool __ends_with_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		auto __dist1 = [&]() {
			if constexpr (std::sized_sentinel_for<_Sentinel1_, _Iterator1_>) {
				return __last1 - __first1;
			} else {
				return std::ranges::distance(__first1, __last1);
			}
		}();

		auto __dist2 = [&]() {
			if constexpr (std::sized_sentinel_for<_Sentinel2_, _Iterator2_>) {
				return __last2 - __first2;
			} else {
				return std::ranges::distance(__first2, __last2);
			}
		}();

		if (__dist2 > __dist1) return false;

		std::advance(__first1, __dist1 - __dist2);

		using _Value_ = std::iter_value_t<_Iterator1_>;

		auto __work = __impl(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work);
	}
};

constexpr inline auto ends_with  = raze::options::function_with_traits<_Ends_with>;

__RAZE_ALGORITHM_NAMESPACE_END