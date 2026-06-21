#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Mismatch : _Traits_ {
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
		_Iterator1_ _result_iterator1;
		_Iterator2_ _result_iterator2;

		constexpr explicit __impl(_Iterator1_ __it1, _Sentinel1_ __sent1, _Iterator2_ __it2,
			_Sentinel2_ __sent2, _Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) noexcept :
			_iterator1(__it1), _sentinel1(__sent1), _iterator2(__it2), _sentinel2(__sent2),
			_predicate(__pred), _proj1(__proj1), _proj2(__proj2),
			_result_iterator1(__it1), _result_iterator2(__it2)
		{}

		template <class _Tag_>
		constexpr raze_always_inline bool operator()(_Tag_) noexcept {
			if (_iterator1 == _sentinel1 || _iterator2 == _sentinel2) {
				_result_iterator1 = _iterator1;
				_result_iterator2 = _iterator2;
				return true;
			}

			if (!_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
				_result_iterator1 = _iterator1;
				_result_iterator2 = _iterator2;
				return true;
			}

			++_iterator1;
			++_iterator2;
			return false;
		}

		raze_always_inline constexpr std::ranges::in_in_result<_Iterator1_, _Iterator2_> result() const noexcept {
			return { _result_iterator1, _result_iterator2 };
		}
	};

	template <class _Tag_>
	struct __vectorized_equal {
		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline std::ranges::in_in_result<_Iterator1_, _Iterator2_> operator()(_Iterator1_ __first1,
			_Sentinel1_ __sentinel1, _Iterator2_ __first2, _Sentinel2_ __sentinel2, 
			_Predicate_ __predicate, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first1 != __sentinel1 && __first2 != __sentinel2; ++__first1, (void)++__first2)
				if (!__predicate(__proj1(*__first1), __proj2(*__first2)))
					break;

			return { __first1, __first2 };
		}

		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline std::ranges::in_in_result<_Iterator1_, _Iterator2_> operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator1_ __first1, _Sentinel1_ __sentinel1, 
			_Iterator2_ __first2, _Sentinel2_ __sentinel2, _Predicate_ __predicate, 
			_Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr1 = std::to_address(__first1);
			auto* __ptr2 = std::to_address(__first2);

			raze_assume(__ptr1 != nullptr);
			raze_assume(__ptr2 != nullptr);
			
			const auto __aligned_end = __bytes_pointer_offset(__ptr1, __aligned_size);

			do {
				const auto __mask = __predicate(__proj1(vx::load<_Tag_>(__ptr1)), __proj2(vx::load<_Tag_>(__ptr2)));
				if (!vx::all_of(__mask)) {
					const auto __mismatch = vx::find_first_not_set[vx::not_null](__mask);
					__seek_possibly_wrapped_iterator(__first1, __ptr1 + __mismatch);
					__seek_possibly_wrapped_iterator(__first2, __ptr2 + __mismatch);
					return { __first1, __first2 };
				}

				__advance_bytes(__ptr1, sizeof(_Tag_));
				__advance_bytes(__ptr2, sizeof(_Tag_));
			} while (__ptr1 != __aligned_end);

			for (; __tail_size != 0; ++__ptr1, ++__ptr2, __tail_size -= sizeof(typename _Tag_::value_type))
				if (!__predicate(__proj1(*__ptr1), __proj2(*__ptr2)))
					break;

			__seek_possibly_wrapped_iterator(__first1, __ptr1);
			__seek_possibly_wrapped_iterator(__first2, __ptr2);

			return { __first1, __first2 };
		}	

		template <sizetype _AlignedSize_, sizetype _TailSize_, class _Iterator1_, 
			class _Sentinel1_, class _Iterator2_, class _Sentinel2_, class _Predicate_,
			class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline std::ranges::in_in_result<_Iterator1_, _Iterator2_> operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator1_ __first1, _Sentinel1_ __sentinel1, 
			_Iterator2_ __first2, _Sentinel2_ __sentinel2, _Predicate_ __predicate, 
			_Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr1 = std::to_address(__first1);
			auto* __ptr2 = std::to_address(__first2);

			raze_assume(__ptr1 != nullptr);
			raze_assume(__ptr2 != nullptr);
			
			constexpr auto __iterations = _AlignedSize_ / sizeof(_Tag_);
			auto __left = __iterations;

			do {
				const auto __mask = __predicate(__proj1(vx::load<_Tag_>(__ptr1)), __proj2(vx::load<_Tag_>(__ptr2)));
				if (!vx::all_of(__mask)) {
					const auto __mismatch = vx::find_first_not_set[vx::not_null](__mask);
					__seek_possibly_wrapped_iterator(__first1, __ptr1 + __mismatch);
					__seek_possibly_wrapped_iterator(__first2, __ptr2 + __mismatch);
					return { __first1, __first2 };
				}

				__advance_bytes(__ptr1, sizeof(_Tag_));
				__advance_bytes(__ptr2, sizeof(_Tag_));
			} while (--__left);
			
			if constexpr (_TailSize_ != 0) {
				constexpr auto __tail_iterations = _TailSize_ / sizeof(_Tag_::value_type);
				auto __tail_left = __tail_iterations;

				do {
					if (!__predicate(__proj1(*__ptr1), __proj2(*__ptr2)))
						break;

					++__ptr1;
					++__ptr2;
				} while (--__tail_left);
			}

			__seek_possibly_wrapped_iterator(__first1, __ptr1);
			__seek_possibly_wrapped_iterator(__first2, __ptr2);

			return { __first1, __first2 };
		}
	};

	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::in_in_result<_Iterator1_, _Iterator2_> operator()(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<_Iterator1_, _Sentinel1_, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __result = __mismatch_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_sentinel<_Iterator1_>(std::move(__last1)),
			type_traits::__ranges_unwrap_iterator<_Sentinel2_>(std::move(__first2)), 
			type_traits::__ranges_unwrap_sentinel<_Iterator2_>(std::move(__last2)), 
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));

		__seek_possibly_wrapped_iterator(__first1, __result.in1);
		__seek_possibly_wrapped_iterator(__first2, __result.in2);

		return { __first1, __first2 };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::in_in_result<std::ranges::borrowed_iterator_t<_Range1_>, 
		std::ranges::borrowed_iterator_t<_Range2_>> operator()(_Range1_&& __range1, _Range2_&& __range2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires((!constexpr_sized_range<_Range1_> || !constexpr_sized_range<_Range2_>) &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __first1 = std::ranges::begin(__range1);
		auto __first2 = std::ranges::begin(__range2);

		auto __result = __mismatch_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__first1)),
			type_traits::__unchecked_end(__range1), type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::move(__first2)), 
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));

		__seek_possibly_wrapped_iterator(__first1, __result.in1);
		__seek_possibly_wrapped_iterator(__first2, __result.in2);

		return { __first1, __first2 };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::in_in_result<std::ranges::borrowed_iterator_t<_Range1_>,
		std::ranges::borrowed_iterator_t<_Range2_>> operator()(_Range1_&& __range1, _Range2_&& __range2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(constexpr_sized_range<_Range1_> && constexpr_sized_range<_Range2_> &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		constexpr auto __min_size = std::min(__range_constexpr_size<_Range1_>(), __range_constexpr_size<_Range2_>());

		auto __first1 = std::ranges::begin(__range1);
		auto __first2 = std::ranges::begin(__range2);

		auto __result = __mismatch_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__first1)),
			type_traits::__unchecked_end(__range1), type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::move(__first2)),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2), std::integral_constant<sizetype, __min_size>{});

		__seek_possibly_wrapped_iterator(__first1, __result.in1);
		__seek_possibly_wrapped_iterator(__first2, __result.in2);

		return { __first1, __first2 };
	}
private:
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	constexpr raze_always_inline std::ranges::in_in_result<_Iterator1_, _Iterator2_> __mismatch_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (std::same_as<_Value1_, _Value2_> &&  std::contiguous_iterator<_Iterator1_> && std::contiguous_iterator<_Iterator2_> &&
			vectorizable_binary_predicate<_Predicate_, _Iterator1_, _Iterator2_> &&
			vectorizable_projection<_Projection1_, _Iterator1_> && vectorizable_projection<_Projection2_, _Iterator2_>)
		{
			if not consteval {
				const auto __size = std::min(algorithm::distance(__first1, __last1), algorithm::distance(__first2, __last2));
				return vx::__dispatch_sized_impl<__vectorized_equal, _Value1_, std::ranges::in_in_result<_Iterator1_, _Iterator2_>>(
					__size * sizeof(_Value1_), __first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}

	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_, sizetype _Size_>
	constexpr raze_always_inline std::ranges::in_in_result<_Iterator1_, _Iterator2_> __mismatch_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2,
		std::integral_constant<sizetype, _Size_>) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (std::same_as<_Value1_, _Value2_> && std::contiguous_iterator<_Iterator1_> && std::contiguous_iterator<_Iterator2_> && 
			vectorizable_binary_predicate<_Predicate_, _Iterator1_, _Iterator2_> &&
			vectorizable_projection<_Projection1_, _Iterator1_> && vectorizable_projection<_Projection2_, _Iterator2_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_equal, _Value1_, std::ranges::in_in_result<_Iterator1_, _Iterator2_>>(
					std::integral_constant<sizetype, _Size_ * sizeof(_Value1_)>{}, 
					__first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}
};

constexpr inline auto mismatch = raze::options::function_with_traits<_Mismatch>;

__RAZE_ALGORITHM_NAMESPACE_END
