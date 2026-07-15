#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>
#include <src/raze/algorithm/memory/SwapRanges.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Swap_ranges : _Traits_ {
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_>
	struct __impl {
		_Iterator1_ _iterator1;
		_Sentinel1_ _sentinel1;
		_Iterator2_ _iterator2;
		_Sentinel2_ _sentinel2;

		constexpr explicit __impl(_Iterator1_ __it1, _Sentinel1_ __sent1,
			_Iterator2_ __it2, _Sentinel2_ __sent2) noexcept :
			_iterator1(__it1), _sentinel1(__sent1), _iterator2(__it2), _sentinel2(__sent2)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_iterator1 == _sentinel1 || _iterator2 == _sentinel2) return true;
			std::ranges::iter_swap(_iterator1, _iterator2);
			++_iterator1;
			++_iterator2;
			return false;
		}

		constexpr raze_always_inline std::ranges::swap_ranges_result<_Iterator1_, _Iterator2_> result() const noexcept {
			return { _iterator1, _iterator2 };
		}
	};

	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_>
	constexpr raze_always_inline std::ranges::swap_ranges_result<_Iterator1_, _Iterator2_> operator()(
		_Iterator1_ __first1, _Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2) const noexcept
			requires(std::indirectly_swappable<_Iterator1_, _Iterator2_>)
	{
		auto __r = __swap_ranges_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_sentinel<_Iterator1_>(std::move(__last1)),
			type_traits::__ranges_unwrap_iterator<_Sentinel2_>(std::move(__first2)),
			type_traits::__ranges_unwrap_sentinel<_Iterator2_>(std::move(__last2)));

		__seek_possibly_wrapped_iterator(__first1, std::move(__r.in1));
		__seek_possibly_wrapped_iterator(__first2, std::move(__r.in2));

		return { std::move(__first1), std::move(__first2) };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_>
	constexpr raze_always_inline std::ranges::swap_ranges_result<
		std::ranges::iterator_t<_Range1_>, std::ranges::iterator_t<_Range2_>> operator()(
		_Range1_&& __range1, _Range2_&& __range2) const noexcept
			requires(!constexpr_sized_range<_Range1_> && !constexpr_sized_range<_Range2_> &&
				std::indirectly_swappable<std::ranges::iterator_t<_Range1_>, std::ranges::iterator_t<_Range2_>>)
	{
		auto __begin1 = std::ranges::begin(__range1);
		auto __begin2 = std::ranges::begin(__range2);

		auto __r = __swap_ranges_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__begin1)),
			type_traits::__unchecked_end(__range1),
			type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::move(__begin2)),
			type_traits::__unchecked_end(__range2));

		__seek_possibly_wrapped_iterator(__begin1, std::move(__r.in1));
		__seek_possibly_wrapped_iterator(__begin2, std::move(__r.in2));

		return { std::move(__begin1), std::move(__begin2) };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_>
	constexpr raze_always_inline std::ranges::swap_ranges_result<
		std::ranges::iterator_t<_Range1_>, std::ranges::iterator_t<_Range2_>> operator()(
		_Range1_&& __range1, _Range2_&& __range2) const noexcept
			requires(constexpr_sized_range<_Range1_> && constexpr_sized_range<_Range2_> &&
				std::indirectly_swappable<std::ranges::iterator_t<_Range1_>, std::ranges::iterator_t<_Range2_>>)
	{
		constexpr auto __size1 = __range_constexpr_size<_Range1_>();
		constexpr auto __size2 = __range_constexpr_size<_Range2_>();
		constexpr auto __min_size = (__size1 < __size2) ? __size1 : __size2;

		auto __begin1 = std::ranges::begin(__range1);
		auto __begin2 = std::ranges::begin(__range2);

		auto __r = __swap_ranges_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__begin1)),
			type_traits::__unchecked_end(__range1),
			type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::move(__begin2)),
			type_traits::__unchecked_end(__range2),
			std::integral_constant<sizetype, __min_size>{});

		__seek_possibly_wrapped_iterator(__begin1, std::move(__r.in1));
		__seek_possibly_wrapped_iterator(__begin2, std::move(__r.in2));

		return { std::move(__begin1), std::move(__begin2) };
	}

private:
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_>
	constexpr raze_always_inline std::ranges::swap_ranges_result<_Iterator1_, _Iterator2_> __swap_ranges_unchecked(
		_Iterator1_ __first1, _Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());
		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() &&  std::contiguous_iterator<_Iterator1_> 
			&& std::contiguous_iterator<_Iterator2_> && std::same_as<_Value1_, _Value2_> && std::is_trivially_copyable_v<_Value1_>)
		{
			if not consteval {
				auto* __f1_ptr = std::to_address(__first1);
				auto* __f2_ptr = std::to_address(__first2);

				const auto __offset = algorithm::__swap_ranges[_Traits_::traits()](__f1_ptr, std::to_address(__last1),
					__f2_ptr, std::to_address(__last2));

				__seek_possibly_wrapped_iterator(__first1, __bytes_pointer_offset(__f1_ptr, __offset));
				__seek_possibly_wrapped_iterator(__first2, __bytes_pointer_offset(__f2_ptr, __offset));

				return std::ranges::swap_ranges_result(__first1, __first2);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first1, __last1, __first2, __last2));
	}

	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_, sizetype _Size_>
	constexpr raze_always_inline std::ranges::swap_ranges_result<_Iterator1_, _Iterator2_> __swap_ranges_unchecked(
		_Iterator1_ __first1, _Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());
		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator1_> 
			&& std::contiguous_iterator<_Iterator2_> && std::same_as<_Value1_, _Value2_> && std::is_trivially_copyable_v<_Value1_>)
		{
			if not consteval {
				auto* __f1_ptr = std::to_address(__first1);
				auto* __f2_ptr = std::to_address(__first2);

				const auto __offset = algorithm::__swap_ranges[_Traits_::traits()](__f1_ptr, std::to_address(__last1),
					__f2_ptr, std::to_address(__last2), std::integral_constant<sizetype, _Size_ * sizeof(_Value1_)>{});

				__seek_possibly_wrapped_iterator(__first1, __bytes_pointer_offset(__f1_ptr, __offset));
				__seek_possibly_wrapped_iterator(__first2, __bytes_pointer_offset(__f2_ptr, __offset));

				return std::ranges::swap_ranges_result(__first1, __first2);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first1, __last1, __first2, __last2));
	}
};

constexpr inline auto swap_ranges = raze::options::function_with_traits<_Swap_ranges>;

__RAZE_ALGORITHM_NAMESPACE_END