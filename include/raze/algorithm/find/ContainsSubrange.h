#pragma once 

#include <raze/algorithm/find/Equal.h>
#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Contains_subrange : _Traits_ {
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
		bool _found = false;

		constexpr explicit __impl(_Iterator1_ __it1, _Sentinel1_ __sent1, _Iterator2_ __it2,
			_Sentinel2_ __sent2, _Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) noexcept :
			_iterator1(__it1), _sentinel1(__sent1), _iterator2(__it2), _sentinel2(__sent2),
			_predicate(__pred), _proj1(__proj1), _proj2(__proj2),
			_start_iterator1(__it1), _start_iterator2(__it2)
		{}

		template <class _Tag_>
		constexpr raze_always_inline bool operator()(_Tag_) noexcept {
			if (_iterator2 == _sentinel2) {
				_found = true;
				return true;
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

		raze_always_inline constexpr bool result() const noexcept {
			return _found;
		}
	};

	template <class _Tag_>
	struct __vectorized_contains_subrange {
		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline bool operator()(_Iterator1_ __first1,
			_Sentinel1_ __sentinel1, _Iterator2_ __first2, _Sentinel2_ __sentinel2, 
			_Predicate_ __predicate, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept
		{
			for (;; ++__first1) {
				auto __it1 = __first1;
				for (auto __it2 = __first2;; ++__it1, ++__it2) {
					if (__it2 == __sentinel2) return true;
					if (__it1 == __sentinel1) return false;
					if (!__predicate(__proj1(*__it1), __proj2(*__it2))) break;
				}
			}
		}

		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline bool operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator1_ __first1, _Sentinel1_ __sentinel1, 
			_Iterator2_ __first2, _Sentinel2_ __sentinel2, _Predicate_ __predicate, 
			_Projection1_ __proj1, _Projection2_ __proj2) const noexcept
		{
			using _Value_ = std::iter_value_t<_Iterator1_>;

			auto* __haystack = std::to_address(__first1);
			auto* __needle = std::to_address(__first2);

			raze_assume(__haystack != nullptr);
			raze_assume(__needle != nullptr);
			
			const auto __aligned_end = __bytes_pointer_offset(__haystack, __aligned_size);
			const auto __needle_length = algorithm::distance(__first2, __sentinel2);

			const auto __needle_first = __needle[0];
			const auto __needle_last = __needle[__needle_length - 1];

			const auto __needle_bytes = sizeof(_Value_) * __needle_length;
			const auto __last_offset = __needle_bytes - sizeof(_Value_);
			const auto __haystack_bytes = __aligned_size + __tail_size;

			auto __processed_bytes = sizetype(0);

			while (__processed_bytes < __aligned_size && (__haystack_bytes - __processed_bytes) >= (__last_offset + sizeof(_Tag_))) {
				const auto __equal_first = __predicate(__proj1(vx::load<_Tag_>(__haystack)), __proj2(__needle_first));

				if (vx::none_of(__equal_first)) {
					__processed_bytes += sizeof(_Tag_);
					__advance_bytes(__haystack, sizeof(_Tag_));
					continue;
				}

				const auto __equal_last = __predicate(__proj1(vx::load<_Tag_>(__bytes_pointer_offset(__haystack, __last_offset))), __proj2(__needle_last));
				auto __combined = __equal_first & __equal_last;

				if (vx::any_of(__combined)) {
					do {
						const auto __first_set = vx::find_first_set[vx::not_null](__combined);

						const auto __match_bytes = __first_set * sizeof(_Value_) + sizeof(_Value_);
						const auto __main_match = __bytes_pointer_offset(__haystack, __match_bytes);

						if (memcmp(__main_match, __bytes_pointer_offset(__needle, sizeof(_Value_)), __needle_bytes - 2 * sizeof(_Value_)) == 0)
							return true;

						__combined = vx::clear_first[vx::not_null](__combined);
					} while (vx::any_of(__combined));
				}

				__processed_bytes += sizeof(_Tag_);
				__advance_bytes(__haystack, sizeof(_Tag_));
			}

			const auto __remaining_bytes = __haystack_bytes - __processed_bytes;

			if (__remaining_bytes >= __needle_bytes) {
				__seek_possibly_wrapped_iterator(__first1, __haystack);
				return (*this)(__first1, __sentinel1, __first2, __sentinel2, __predicate, __proj1, __proj2);
			}
			else return false;
		}
	};

	template <std::input_iterator _Iterator1_, std::sentinel_for<_Iterator1_> _Sentinel1_,
		std::input_iterator _Iterator2_, std::sentinel_for<_Iterator2_> _Sentinel2_,
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(std::indirectly_comparable<_Iterator1_, _Sentinel1_, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __contains_subrange_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_sentinel<_Iterator1_>(std::move(__last1)),
			type_traits::__ranges_unwrap_iterator<_Sentinel2_>(std::move(__first2)), 
			type_traits::__ranges_unwrap_sentinel<_Iterator2_>(std::move(__last2)), 
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __range1, _Range2_&& __range2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires((!constexpr_sized_range<_Range1_> || !constexpr_sized_range<_Range2_>) &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __contains_subrange_unchecked(type_traits::__unchecked_begin(__range1),
			type_traits::__unchecked_end(__range1), type_traits::__unchecked_begin(__range2),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_, 
		class _Predicate_ = std::equal_to<>, class _Projection1_ = std::identity,
		class _Projection2_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range1_&& __range1, _Range2_&& __range2,
		_Predicate_ __pred = {}, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(constexpr_sized_range<_Range1_> && constexpr_sized_range<_Range2_> &&
				std::indirectly_comparable<std::ranges::iterator_t<_Range1_>,
					std::ranges::iterator_t<_Range2_>, _Predicate_, _Projection1_, _Projection2_>)
	{
		return __search_unchecked(type_traits::__unchecked_begin(__range1),
			type_traits::__unchecked_end(__range1), type_traits::__unchecked_begin(__range2),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2), std::integral_constant<sizetype, __range_constexpr_size<_Range1_>()>{},
			std::integral_constant<sizetype, __range_constexpr_size<_Range2_>()>{});
	}
private:
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	constexpr raze_always_inline bool __contains_subrange_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());


		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::same_as<_Value1_, _Value2_> &&  std::contiguous_iterator<_Iterator1_> && std::contiguous_iterator<_Iterator2_> &&
			vectorizable_binary_predicate<_Predicate_, _Iterator1_, _Iterator2_> &&
			vectorizable_projection<_Projection1_, _Iterator1_> && vectorizable_projection<_Projection2_, _Iterator2_>)
		{
			if not consteval {
				const auto __needle_size = algorithm::distance(__first2, __last2);
				if (__needle_size == 0) return true;

				const auto __haystack_size = algorithm::distance(__first1, __last1);
				if (__needle_size == __haystack_size) return equal(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
				if (__needle_size > __haystack_size) return false;
				if (__needle_size == 1) return find_if(__first1, __last1, [&__pred, &__first2] (auto __x)
					raze_always_inline_lambda { return __pred(__x, *__first2); }) != __last1;
				return vx::__dispatch_sized_impl<__vectorized_contains_subrange, _Value1_, bool>(
					__haystack_size * sizeof(_Value1_), __first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(
			__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}

	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_, sizetype _HaystackSize_,
		sizetype _NeedleSize_>
	constexpr raze_always_inline bool __contains_subrange_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2,
		std::integral_constant<sizetype, _HaystackSize_>,
		std::integral_constant<sizetype, _NeedleSize_>) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());

		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::same_as<_Value1_, _Value2_> 
			&& std::contiguous_iterator<_Iterator1_> && std::contiguous_iterator<_Iterator2_> &&
			vectorizable_binary_predicate<_Predicate_, _Iterator1_, _Iterator2_> &&
			vectorizable_projection<_Projection1_, _Iterator1_> && vectorizable_projection<_Projection2_, _Iterator2_>)
		{
			if not consteval {
				if constexpr (_NeedleSize_ == 0) return true;
				else if constexpr (_NeedleSize_ == _HaystackSize_) return equal(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
				else if constexpr (_NeedleSize_ > _HaystackSize_) return false;
				else if constexpr (_NeedleSize_ == 1) return find_if(__first1, __last1, [&__pred, &__first2] (auto __x) 
					raze_always_inline_lambda { return __pred(__x, *__first2); }) != __last1;
				else return vx::__dispatch_sized_impl<__vectorized_contains_subrange, _Value1_, bool>(
					std::integral_constant<sizetype, _HaystackSize_ * sizeof(_Value1_)>{}, __first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(
			__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}
};

constexpr inline auto contains_subrange = raze::options::function_with_traits<_Contains_subrange>;

__RAZE_ALGORITHM_NAMESPACE_END
