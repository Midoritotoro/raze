#pragma once 

#include <raze/vx/Algorithm.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/UncheckedAlgorithms.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Equal : _Traits_ {
	template <source _Source1_, source _Source2_,
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
		return __equal_unchecked(traits::__uiter<_Sentinel1_>(std::move(__first1)),
			traits::__usent<_Iterator1_>(std::move(__last1)),
			traits::__uiter<_Sentinel2_>(std::move(__first2)), 
			traits::__usent<_Iterator2_>(std::move(__last2)), 
			traits::__fwd_fn(__pred), traits::__fwd_fn(__proj1),
			traits::__fwd_fn(__proj2));
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
		return __equal_unchecked(traits::__r_uiter<_Range1_>(std::ranges::begin(__range1)),
			traits::__uend(__range1), traits::__r_uiter<_Range2_>(std::ranges::begin(__range2)), 
			traits::__uend(__range2), traits::__fwd_fn(__pred), traits::__fwd_fn(__proj1),
			traits::__fwd_fn(__proj2));
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
		else return __equal_unchecked(traits::__r_uiter<_Range1_>(std::ranges::begin(__range1)),
			traits::__uend(__range1), traits::__r_uiter<_Range2_>(std::ranges::begin(__range2)),
			traits::__uend(__range2), traits::__fwd_fn(__pred), traits::__fwd_fn(__proj1),
			traits::__fwd_fn(__proj2), std::integral_constant<sizetype, __range_constexpr_size<_Range1_>()>{});
	}
private:
	__raze_define_kernel_dispatch()
};

constexpr inline auto equal = raze::options::function_with_traits<_Equal>;

__RAZE_ALGORITHM_NAMESPACE_END
