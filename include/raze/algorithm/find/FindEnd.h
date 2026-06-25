#pragma once 

#include <raze/algorithm/find/Search.h>
#include <raze/algorithm/find/FindLast.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Find_end : _Traits_ {
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
		_Iterator1_ _start_iterator1;
		_Iterator2_ _start_iterator2;

		_Iterator1_ _best_start;
		_Iterator1_ _best_end;
		bool _found;
		bool _empty_pattern;

		constexpr explicit __impl(_Iterator1_ __it1, _Sentinel1_ __sent1, _Iterator2_ __it2,
			_Sentinel2_ __sent2, _Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) noexcept :
			_iterator1(__it1), _sentinel1(__sent1), _iterator2(__it2), _sentinel2(__sent2),
			_predicate(__pred), _proj1(__proj1), _proj2(__proj2),
			_start_iterator1(__it1), _start_iterator2(__it2),
			_best_start(__sent1), _best_end(__sent1),
			_found(false), _empty_pattern(__it2 == __sent2)
		{}

		template <class _Tag_>
		constexpr raze_always_inline bool operator()(_Tag_) noexcept {
			if (_empty_pattern) [[unlikely]] {
				return true;
			}

			if (_iterator2 == _sentinel2) {
				_best_start = _start_iterator1;
				_best_end = _iterator1;
				_found = true;

				++_start_iterator1;
				_iterator1 = _start_iterator1;
				_iterator2 = _start_iterator2;
				return false;
			}

			if (_iterator1 == _sentinel1) {
				return true;
			}

			if (_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
				++_iterator1;
				++_iterator2;
				return false;
			}

			++_start_iterator1;
			_iterator1 = _start_iterator1;
			_iterator2 = _start_iterator2;
			return false;
		}

		raze_always_inline constexpr std::ranges::subrange<_Iterator1_> result() const noexcept {
			if (_found) return { _best_start, _best_end };
			return { _sentinel1, _sentinel1 };
		}
	};

	template <class _Tag_>
	struct __vectorized_find_end {
		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline std::ranges::subrange<_Iterator1_> operator()(_Iterator1_ __first1,
			_Sentinel1_ __sentinel1, _Iterator2_ __first2, _Sentinel2_ __sentinel2, 
			_Predicate_ __predicate, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept
		{
			if (__first2 == __sentinel2) {
				auto __last = std::ranges::next(__first1, __sentinel1);
				return { __last, __last };
			}

			auto __result = std::ranges::search(std::move(__first1), __sentinel1, __first2, __sentinel2, __predicate, __proj1, __proj2);

			if (__result.empty())
				return __result;

			for (;;) {
				auto __new_result = std::ranges::search(std::next(__result.begin()), __sentinel1, __first2, __sentinel2, __predicate, __proj1, __proj2);
				if (__new_result.empty()) return __result;
				else __result = std::move(__new_result);
			}
		}

		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline std::ranges::subrange<_Iterator1_> operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator1_ __first1, _Sentinel1_ __sentinel1, 
			_Iterator2_ __first2, _Sentinel2_ __sentinel2, _Predicate_ __predicate, 
			_Projection1_ __proj1, _Projection2_ __proj2) const noexcept
		{
			using _Value_ = std::iter_value_t<_Iterator1_>;

			auto* __haystack_last = __bytes_pointer_offset(std::to_address(__first1), __tail_size + __aligned_size);
			auto* __needle = std::to_address(__first2);

			const auto __cached_last = __haystack_last;

			raze_assume(__haystack_last != nullptr);
			raze_assume(__needle != nullptr);
			
			const auto __aligned_end = __bytes_pointer_offset(__haystack_last, __aligned_size);
			const auto __needle_length = algorithm::distance(__first2, __sentinel2);

			const auto __needle_first = __needle[0];
			const auto __needle_last = __needle[__needle_length - 1];

			const auto __needle_bytes = sizeof(_Value_) * __needle_length;
			const auto __last_offset = __needle_bytes;
			const auto __haystack_bytes = __aligned_size + __tail_size;

			auto __processed_bytes = sizetype(0);

			while (__processed_bytes < __aligned_size && (__haystack_bytes - __processed_bytes) >= (__last_offset + sizeof(_Tag_))) {
				__rewind_bytes(__haystack_last, sizeof(_Tag_));
				__processed_bytes += sizeof(_Tag_);

				const auto __equal_last = __predicate(__proj1(vx::load<_Tag_>(__haystack_last)), __proj2(__needle_last));

				if (vx::none_of(__equal_last)) continue;

				const auto __sequence_start = __bytes_pointer_offset(__haystack_last, -(__last_offset - sizeof(_Value_)));
				const auto __equal_first = __predicate(__proj1(vx::load<_Tag_>(__sequence_start)), __proj2(__needle_first));

				auto __combined = __equal_first & __equal_last;

				if (vx::any_of(__combined)) {
					do {
						const auto __offset_bytes = (__combined.size() - vx::find_last_set[vx::not_null](__combined)) * sizeof(_Value_);
						const auto __main_match = __bytes_pointer_offset(__sequence_start, __offset_bytes);

						if (memcmp(__main_match, __bytes_pointer_offset(__needle, sizeof(_Value_)), __needle_bytes - 2 * sizeof(_Value_)) == 0) {
							__seek_possibly_wrapped_iterator(__first1, __main_match - 1);
							return { __first1, __first1 + __needle_length };
						}

						__combined = vx::clear_last[vx::not_null](__combined);
					} while (vx::any_of(__combined));
				}
			}

			const auto __remaining_bytes = __haystack_bytes - __processed_bytes;

			if (__remaining_bytes < __needle_bytes)
				return { __cached_last, __cached_last };

			const auto __remaining_length = __remaining_bytes / sizeof(_Value_);
			const auto __haystack_first = std::to_address(__first1);

			const auto __scalar_result = (*this)(__haystack_first, __haystack_first + __remaining_length,
				__needle, __needle + __needle_length, __predicate, __proj1, __proj2);

			if (__scalar_result.begin() == __haystack_first + __remaining_length) return { __cached_last, __cached_last };
			return __scalar_result;
		}
	};

	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::subrange<_Iterator1_> operator()(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<_Iterator1_, _Sentinel1_, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __result = __find_end_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_sentinel<_Iterator1_>(std::move(__last1)),
			type_traits::__ranges_unwrap_iterator<_Sentinel2_>(std::move(__first2)), 
			type_traits::__ranges_unwrap_sentinel<_Iterator2_>(std::move(__last2)), 
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));

		__seek_possibly_wrapped_iterator(__first1, __result.begin());
		__seek_possibly_wrapped_iterator(__last1, __result.end());

		return { __first1, __last1 };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::subrange<std::ranges::borrowed_iterator_t<_Range1_>> operator()(_Range1_&& __range1, _Range2_&& __range2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires((!constexpr_sized_range<_Range1_> || !constexpr_sized_range<_Range2_>) &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __first1 = std::ranges::begin(__range1);
		auto __last1 = std::ranges::end(__range1);

		auto __result = __find_end_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_range_sentinel<_Range1_>(std::move(__last1)), type_traits::__unchecked_begin(__range2),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));

		__seek_possibly_wrapped_iterator(__first1, __result.begin());
		__seek_possibly_wrapped_iterator(__last1, __result.end());

		return { __first1, __last1 };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::subrange<std::ranges::borrowed_iterator_t<_Range1_>> operator()(_Range1_&& __range1, _Range2_&& __range2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(constexpr_sized_range<_Range1_> && constexpr_sized_range<_Range2_> &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		auto __first1 = std::ranges::begin(__range1);
		auto __last1 = std::ranges::end(__range1);

		auto __result = __find_end_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_range_sentinel<_Range1_>(std::move(__last1)), type_traits::__unchecked_begin(__range2),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2), std::integral_constant<sizetype, __range_constexpr_size<_Range1_>()>{},
			std::integral_constant<sizetype, __range_constexpr_size<_Range2_>()>{});

		__seek_possibly_wrapped_iterator(__first1, __result.begin());
		__seek_possibly_wrapped_iterator(__last1, __result.end());

		return { __first1, __last1 };
	}
private:
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	constexpr raze_always_inline std::ranges::subrange<_Iterator1_> __find_end_unchecked(_Iterator1_ __first1,
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
				const auto __needle_size = algorithm::distance(__first2, __last2);
				if (__needle_size == 0) return { __last1, __last1 };

				const auto __haystack_size = algorithm::distance(__first1, __last1);
				if (__needle_size == __haystack_size) return equal(__first1,
					__last1, __first2, __last2, __pred, __proj1, __proj2) ? std::ranges::subrange { __first1, __last1 } : std::ranges::subrange { __last1, __last1 };
				if (__needle_size > __haystack_size) return { __last1, __last1 };
				if (__needle_size == 1) {
					const auto __found = find_last_if(__first1, __last1, [&__pred, &__first2](auto __x)
						raze_always_inline_lambda{ return __pred(__x, *__first2); });

					return { __found.begin(), __found.begin() == __last1 ? __found.begin() : std::ranges::next(__found.begin()) };
				}
				return vx::__dispatch_sized_impl<__vectorized_find_end, _Value1_, std::ranges::subrange<_Iterator1_>>(
					__haystack_size * sizeof(_Value1_), __first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(
			__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}

	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_, sizetype _HaystackSize_,
		sizetype _NeedleSize_>
	constexpr raze_always_inline std::ranges::subrange<_Iterator1_> __find_end_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2,
		std::integral_constant<sizetype, _HaystackSize_>,
		std::integral_constant<sizetype, _NeedleSize_>) const noexcept
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
				if constexpr (_NeedleSize_ == 0) return { __last1, __last1 };
				else if constexpr (_NeedleSize_ == _HaystackSize_) return equal(__first1,
					__last1, __first2, __last2, __pred, __proj1, __proj2) ? std::ranges::subrange{ __first1, __last1 } : std::ranges::subrange{ __last1, __last1 };
				else if constexpr (_NeedleSize_ > _HaystackSize_) return { __last1, __last1 };
				else if constexpr (_NeedleSize_ == 1) {
					const auto __found = find_last_if(__first1, __last1, [&__pred, &__first2] (auto __x) 
						raze_always_inline_lambda { return __pred(__x, *__first2); });

					return { __found.begin(), __found.begin() == __last1 ? __found.begin() : std::ranges::next(__found.begin()) };
				}
				else {
					return vx::__dispatch_sized_impl<__vectorized_find_end, _Value1_, std::ranges::subrange<_Iterator1_>>(
						std::integral_constant<sizetype, _HaystackSize_ * sizeof(_Value1_)>{}, __first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
				}
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(
			__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}
};

constexpr inline auto find_end = raze::options::function_with_traits<_Find_end>;

__RAZE_ALGORITHM_NAMESPACE_END
