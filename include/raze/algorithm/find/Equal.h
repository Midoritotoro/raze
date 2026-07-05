#pragma once 

#include <raze/algorithm/find/Find.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Equal : _Traits_ {
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
		bool _result = false;

		constexpr explicit __impl(_Iterator1_ __it1, _Sentinel1_ __sent1, _Iterator2_ __it2,
			_Sentinel2_ __sent2, _Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) noexcept:
				_iterator1(__it1), _sentinel1(__sent1), _iterator2(__it2), _sentinel2(__sent2),
				_predicate(__pred), _proj1(__proj1), _proj2(__proj2)
		{}

		template <class _Tag_>
		constexpr raze_always_inline bool operator()(_Tag_) noexcept {
			if (_iterator1 == _sentinel1 || _iterator2 == _sentinel2) {
				_result = (_iterator1 == _sentinel1 && _iterator2 == _sentinel2);
				return true;
			}

			if (!_predicate(_proj1(*_iterator1), _proj2(*_iterator2))) {
				_result = false;
				return true;
			}

			++_iterator1;
			++_iterator2;
			return false;
		}

		raze_always_inline constexpr bool result() const noexcept {
			return _result;
		}
	};

	template <class _Tag_>
	struct __vectorized_equal {
		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline bool operator()(_Iterator1_ __first1,
			_Sentinel1_ __sentinel1, _Iterator2_ __first2, _Sentinel2_ __sentinel2, 
			_Predicate_ __predicate, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first1 != __sentinel1; ++__first1, (void)++__first2)
				if (!__predicate(__proj1(*__first1), __proj2(*__first2)))
					return false;

			return true;
		}

		template <class _Iterator1_, class _Sentinel1_, class _Iterator2_,
			class _Sentinel2_, class _Predicate_, class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline bool operator()(sizetype __aligned_size,
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
				if (!vx::all_of(__mask)) return false;

				__advance_bytes(__ptr1, sizeof(_Tag_));
				__advance_bytes(__ptr2, sizeof(_Tag_));
			} while (__ptr1 != __aligned_end);


			for (; __tail_size != 0; ++__ptr1, (void)++__ptr2, __tail_size -= sizeof(typename _Tag_::value_type))
				if (!__predicate(__proj1(*__ptr1), __proj2(*__ptr2)))
					return false;

			return true;
		}	

		template <sizetype _AlignedSize_, sizetype _TailSize_, class _Iterator1_, 
			class _Sentinel1_, class _Iterator2_, class _Sentinel2_, class _Predicate_,
			class _Projection1_, class _Projection2_>
		raze_nodiscard raze_always_inline bool operator()(std::integral_constant<sizetype, _AlignedSize_>,
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
				if (!vx::all_of(__mask)) return false;

				__advance_bytes(__ptr1, sizeof(_Tag_));
				__advance_bytes(__ptr2, sizeof(_Tag_));
			} while (--__left);

			if constexpr (_TailSize_ != 0) {
				constexpr auto __tail_iterations = _TailSize_ / sizeof(typename _Tag_::value_type);
				auto __tail_left = __tail_iterations;

				do {
					if (!__predicate(__proj1(*__ptr1), __proj2(*__ptr2)))
						return false;

					++__ptr1;
					++__ptr2;
				} while (--__tail_left);
			}

			return true;
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
		return __equal_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
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
		return __equal_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::ranges::begin(__range1)),
			type_traits::__unchecked_end(__range1), type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::ranges::begin(__range2)), 
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
		if constexpr (__range_constexpr_size<_Range1_>() != __range_constexpr_size<_Range2_>()) return false;
		else return __equal_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::ranges::begin(__range1)),
			type_traits::__unchecked_end(__range1), type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::ranges::begin(__range2)),
			type_traits::__unchecked_end(__range2), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2), std::integral_constant<sizetype, __range_constexpr_size<_Range1_>()>{});
	}
private:
	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_>
	constexpr raze_always_inline bool __equal_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());

		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		const auto __first_size = algorithm::distance(__first1, __last1);

		if constexpr (std::random_access_iterator<_Iterator1_> && std::same_as<_Iterator1_, _Sentinel1_> &&
			std::random_access_iterator<_Iterator2_> && std::same_as<_Iterator1_, _Sentinel2_>)
		{
			if (__first_size != algorithm::distance(__first2, __last2)) return false;
		}

		if constexpr (!options::always_scalar<_TraitsType>() && std::same_as<_Value1_, _Value2_> && 
			std::contiguous_iterator<_Iterator1_> && std::contiguous_iterator<_Iterator2_> &&
			vectorizable_binary_predicate<_Predicate_, _Iterator1_, _Iterator2_> &&
			vectorizable_projection<_Projection1_, _Iterator1_> && vectorizable_projection<_Projection2_, _Iterator2_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_equal, _Value1_, bool>(
					__first_size * sizeof(_Value1_), __first1, __last1,
					__first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}

	template <class _Iterator1_, class _Sentinel1_, class _Iterator2_, class _Sentinel2_,
		class _Predicate_, class _Projection1_, class _Projection2_, sizetype _Size_>
	constexpr raze_always_inline bool __equal_unchecked(_Iterator1_ __first1,
		_Sentinel1_ __last1, _Iterator2_ __first2, _Sentinel2_ __last2,
		_Predicate_ __pred, _Projection1_ __proj1, _Projection2_ __proj2,
		std::integral_constant<sizetype, _Size_>) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());

		using _Value1_ = std::iter_value_t<_Iterator1_>;
		using _Value2_ = std::iter_value_t<_Iterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::same_as<_Value1_, _Value2_> && 
			std::contiguous_iterator<_Iterator1_> && std::contiguous_iterator<_Iterator2_> &&
			vectorizable_binary_predicate<_Predicate_, _Iterator1_, _Iterator2_> &&
			vectorizable_projection<_Projection1_, _Iterator1_> && vectorizable_projection<_Projection2_, _Iterator2_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_equal, _Value1_, bool>(
					std::integral_constant<sizetype, _Size_ * sizeof(_Value1_)>{}, 
					__first1, __last1, __first2, __last2, __pred, __proj1, __proj2);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first1, __last1, __first2, __last2, __pred, __proj1, __proj2));
	}
};

constexpr inline auto equal = raze::options::function_with_traits<_Equal>;

__RAZE_ALGORITHM_NAMESPACE_END
