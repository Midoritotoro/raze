#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Transform : _Traits_ {
	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Function_, class _Projection_>
	struct __unary_impl {
		_InputIterator_ _in_iterator;
		_Sentinel_ _in_sentinel;
		_OutIterator_ _out_iterator;
		_Function_ _function;
		_Projection_ _proj;

		constexpr explicit __unary_impl(_InputIterator_ __in_it, _Sentinel_ __in_sent,
			_OutIterator_ __out_it, _Function_ __f, _Projection_ __proj) noexcept:
				_in_iterator(__in_it), _in_sentinel(__in_sent), _out_iterator(__out_it), _function(__f), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_in_iterator == _in_sentinel) return true;
			*_out_iterator++ = _function(_proj(*_in_iterator++));
			return false;
		}

		constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> result() const noexcept {
			return { _in_iterator, _out_iterator };
		}
	};

	template <class _InputIterator1_, class _Sentinel1_, class _InputIterator2_, class _Sentinel2_,
		class _OutIterator_, class _Function_, class _Projection1_, class _Projection2_>
	struct __binary_impl {
		_InputIterator1_ _in_iterator1;
		_Sentinel1_ _in_sentinel1;
		_InputIterator2_ _in_iterator2;
		_Sentinel2_ _in_sentinel2;
		_OutIterator_ _out_iterator;
		_Function_ _function;
		_Projection1_ _proj1;
		_Projection2_ _proj2;

		constexpr explicit __binary_impl(_InputIterator1_ __in1, _Sentinel1_ __sent1, _InputIterator2_ __in2, _Sentinel2_ __sent2,
			_OutIterator_ __out_it, _Function_ __f, _Projection1_ __proj1, _Projection2_ __proj2) noexcept:
			_in_iterator1(__in1), _in_sentinel1(__sent1), _in_iterator2(__in2), _in_sentinel2(__sent2),
			_out_iterator(__out_it), _function(__f), _proj1(__proj1), _proj2(__proj2)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_in_iterator1 == _in_sentinel1 || _in_iterator2 == _in_sentinel2) return true;
			*_out_iterator++ = _function(_proj1(*_in_iterator1++), _proj2(*_in_iterator2++));
			return false;
		}

		constexpr raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> result() const noexcept {
			return { _in_iterator1, _in_iterator2, _out_iterator };
		}
	};

	template <class _Tag_>
	struct __vectorized_unary_transform {
		template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Function_, class _Projection_>
		raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
			_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result, _Function_ __f,
			_Projection_ __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __last; ++__first, ++__result)
				*__result = __f(__proj(*__first));

			return { std::move(__first), std::move(__result) };
		}
	
		template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Function_, class _Projection_>
		raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
			sizetype __aligned_size, sizetype __tail_size, _InputIterator_ __first, _Sentinel_ __last,
			_OutIterator_ __result, _Function_ __f, _Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			raze_assume(__in_ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__in_ptr, __aligned_size);

			do {
				vx::store(__out_ptr, __f(__proj(vx::load<_Tag_>(__in_ptr))));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (__in_ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			for (; __first != __last; ++__first, ++__result)
				*__result = __f(__proj(*__first));

			return { std::move(__first), std::move(__result) };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Function_, class _Projection_>
		raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
			std::integral_constant<sizetype, _AlignedSize_>, std::integral_constant<sizetype, _TailSize_>,
			_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result, _Function_ __f, _Projection_ __proj)
				const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			raze_assume(__in_ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				vx::store(__out_ptr, __f(__proj(vx::load<_Tag_>(__in_ptr))));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			for (; __first != __last; ++__first, ++__result)
				*__result = __f(__proj(*__first));

			return { std::move(__first), std::move(__result) };
		}
	};

	template <class _Tag_>
	struct __vectorized_binary_transform {
		template <class _InputIterator1_, class _Sentinel1_, class _InputIterator2_, class _Sentinel2_,
			class _OutIterator_, class _Function_, class _Projection1_, class _Projection2_>
		raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> operator()(
			_InputIterator1_ __first1, _Sentinel1_ __last1, _InputIterator2_ __first2, _Sentinel2_ __last2,
			_OutIterator_ __result, _Function_ __f, _Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first1 != __last1 && __first2 != __last2; ++__first1, ++__first2, ++__result)
				*__result = __f(__proj1(*__first1), __proj2(*__first2));

			return { std::move(__first1), std::move(__first2), std::move(__result) };
		}

		template <class _InputIterator1_, class _Sentinel1_, class _InputIterator2_, class _Sentinel2_,
			class _OutIterator_, class _Function_, class _Projection1_, class _Projection2_>
		raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> operator()(
			sizetype __aligned_size, sizetype __tail_size, _InputIterator1_ __first1, _Sentinel1_ __last1,
			_InputIterator2_ __first2, _Sentinel2_ __last2, _OutIterator_ __result, _Function_ __f,
			_Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __in1_ptr = std::to_address(__first1);
			auto* __in2_ptr = std::to_address(__first2);
			auto* __out_ptr = std::to_address(__result);

			raze_assume(__in1_ptr != nullptr);
			raze_assume(__in2_ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__in1_ptr, __aligned_size);

			do {
				vx::store(__out_ptr, __f(__proj1(vx::load<_Tag_>(__in1_ptr)), __proj2(vx::load<_Tag_>(__in2_ptr))));
				__advance_bytes(__in1_ptr, __in2_ptr, sizeof(_Tag_));
				__advance_bytes(__out_ptr, sizeof(_Tag_));
			} while (__in1_ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first1, __in1_ptr);
			__seek_possibly_wrapped_iterator(__first2, __in2_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			for (; __first1 != __last1 && __first2 != __last2; ++__first1, ++__first2, ++__result)
				*__result = __f(__proj1(*__first1), __proj2(*__first2));

			return { std::move(__first1), std::move(__first2), std::move(__result) };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InputIterator1_, class _Sentinel1_, class _InputIterator2_, class _Sentinel2_,
			class _OutIterator_, class _Function_, class _Projection1_, class _Projection2_>
		raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> operator()(
			std::integral_constant<sizetype, _AlignedSize_>, std::integral_constant<sizetype, _TailSize_>, _InputIterator1_ __first1,
			_Sentinel1_ __last1, _InputIterator2_ __first2, _Sentinel2_ __last2, _OutIterator_ __result, _Function_ __f,
			_Projection1_ __proj1, _Projection2_ __proj2) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __in1_ptr = std::to_address(__first1);
			auto* __in2_ptr = std::to_address(__first2);
			auto* __out_ptr = std::to_address(__result);

			raze_assume(__in1_ptr != nullptr);
			raze_assume(__in2_ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				vx::store(__out_ptr, __f(__proj1(vx::load<_Tag_>(__in1_ptr)), __proj2(vx::load<_Tag_>(__in2_ptr))));
				__advance_bytes(__in1_ptr, __in2_ptr, sizeof(_Tag_));
				__advance_bytes(__out_ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first1, __in1_ptr);
			__seek_possibly_wrapped_iterator(__first2, __in2_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			for (; __first1 != __last1 && __first2 != __last2; ++__first1, ++__first2, ++__result)
				*__result = __f(__proj1(*__first1), __proj2(*__first2));

			return { std::move(__first1), std::move(__first2), std::move(__result) };
		}
	};

	template <std::input_iterator _InputIterator_, std::sentinel_for<_InputIterator_> _Sentinel_,
		std::weakly_incrementable _OutIterator_, class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result, _Function_ __f, _Projection_ __proj = {}) const noexcept
			requires(std::indirectly_writable<_OutIterator_, std::indirect_result_t<_Function_, std::projected<_InputIterator_, _Projection_>>>)
	{
		auto __r = __transform_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_InputIterator_>(std::move(__last)), 
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__f), type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_,
		class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, _OutIterator_ __result, _Function_ __f, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirectly_writable<_OutIterator_,
				std::indirect_result_t<_Function_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __transform_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__f), type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_,
		class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, _OutIterator_ __result, _Function_ __f, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirectly_writable<_OutIterator_,
				std::indirect_result_t<_Function_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __transform_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__f), type_traits::__pass_function(__proj),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__end, std::move(__r.out));

		return { std::move(__begin), std::move(__end) };
	}

	template <std::input_iterator _InputIterator1_, std::sentinel_for<_InputIterator1_> _Sentinel1_,
		std::input_iterator _InputIterator2_, std::sentinel_for<_InputIterator2_> _Sentinel2_,
		std::weakly_incrementable _OutIterator_, class _Function_, class _Projection1_ = std::identity, 
		class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> operator()(
		_InputIterator1_ __first1, _Sentinel1_ __last1, _InputIterator2_ __first2, _Sentinel2_ __last2, _OutIterator_ __result, 
		_Function_ __f, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept requires(std::indirectly_writable<_OutIterator_,
			std::indirect_result_t<_Function_, std::projected<_InputIterator1_, _Projection1_>, std::projected<_InputIterator2_, _Projection2_>>>)
	{
		auto __r = __binary_transform_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel1_>(std::move(__first1)),
			type_traits::__ranges_unwrap_sentinel<_InputIterator1_>(std::move(__last1)),
			type_traits::__ranges_unwrap_iterator<_Sentinel2_>(std::move(__first2)),
			type_traits::__ranges_unwrap_sentinel<_InputIterator2_>(std::move(__last2)),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__f), type_traits::__pass_function(__proj1),
			type_traits::__pass_function(__proj2));

		__seek_possibly_wrapped_iterator(__first1, std::move(__r.in1));
		__seek_possibly_wrapped_iterator(__first2, std::move(__r.in2));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first1), std::move(__first2), std::move(__result) };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_,
		std::weakly_incrementable _OutIterator_, class _Function_,
		class _Projection1_ = std::identity, class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::binary_transform_result<
		std::ranges::iterator_t<_Range1_>, std::ranges::iterator_t<_Range2_>, _OutIterator_> operator()(
		_Range1_&& __range1, _Range2_&& __range2, _OutIterator_ __result,
		_Function_ __f, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(!constexpr_sized_range<_Range1_> && !constexpr_sized_range<_Range2_> &&
				std::indirectly_writable<_OutIterator_,
					std::indirect_result_t<_Function_,
						std::projected<std::ranges::iterator_t<_Range1_>, _Projection1_>,
						std::projected<std::ranges::iterator_t<_Range2_>, _Projection2_>>>)
	{
		auto __begin1 = std::ranges::begin(__range1);
		auto __begin2 = std::ranges::begin(__range2);

		auto __r = __binary_transform_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__begin1)),
			type_traits::__unchecked_end(__range1),
			type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::move(__begin2)),
			type_traits::__unchecked_end(__range2),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__f),
			type_traits::__pass_function(__proj1), type_traits::__pass_function(__proj2));

		__seek_possibly_wrapped_iterator(__begin1, std::move(__r.in1));
		__seek_possibly_wrapped_iterator(__begin2, std::move(__r.in2));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin1), std::move(__begin2), std::move(__result) };
	}

	template <std::ranges::input_range _Range1_, std::ranges::input_range _Range2_,
		std::weakly_incrementable _OutIterator_, class _Function_,
		class _Projection1_ = std::identity, class _Projection2_ = std::identity>
	constexpr raze_always_inline std::ranges::binary_transform_result<
		std::ranges::iterator_t<_Range1_>, std::ranges::iterator_t<_Range2_>, _OutIterator_> operator()(
		_Range1_&& __range1, _Range2_&& __range2, _OutIterator_ __result,
		_Function_ __f, _Projection1_ __proj1 = {}, _Projection2_ __proj2 = {}) const noexcept
			requires(constexpr_sized_range<_Range1_> && constexpr_sized_range<_Range2_> &&
				std::indirectly_writable<_OutIterator_,
					std::indirect_result_t<_Function_,
						std::projected<std::ranges::iterator_t<_Range1_>, _Projection1_>,
						std::projected<std::ranges::iterator_t<_Range2_>, _Projection2_>>>)
	{
		constexpr auto __size1 = __range_constexpr_size<_Range1_>();
		constexpr auto __size2 = __range_constexpr_size<_Range2_>();
		constexpr auto __min_size = (__size1 < __size2) ? __size1 : __size2;
	
		auto __begin1 = std::ranges::begin(__range1);
		auto __begin2 = std::ranges::begin(__range2);

		auto __r = __binary_transform_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range1_>(std::move(__begin1)),
			type_traits::__unchecked_end(__range1),
			type_traits::__ranges_unwrap_range_iterator<_Range2_>(std::move(__begin2)),
			type_traits::__unchecked_end(__range2),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__f),
			type_traits::__pass_function(__proj1), type_traits::__pass_function(__proj2),
			std::integral_constant<sizetype, __min_size>{});

		__seek_possibly_wrapped_iterator(__begin1, std::move(__r.in1));
		__seek_possibly_wrapped_iterator(__begin2, std::move(__r.in2));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin1), std::move(__begin2), std::move(__result) };
	}

private:
	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Function_, class _Projection_>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> __transform_unchecked(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result, _Function_ __f, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _InValue_ = std::iter_value_t<_InputIterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_InputIterator_> && std::contiguous_iterator<_OutIterator_> &&
			vectorizable_unary_function<_Function_, _InputIterator_> && vectorizable_projection<_Projection_, _InputIterator_> &&
			type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_unary_transform, _InValue_,
					std::ranges::unary_transform_result<_InputIterator_, _OutIterator_>>(
					algorithm::distance(__first, __last) * sizeof(_InValue_),
					__first, __last, __result, __f, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__unary_impl(__first, __last, __result, __f, __proj));
	}

	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Function_, class _Projection_, sizetype _Size_>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> __transform_unchecked(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		_Function_ __f, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _InValue_ = std::iter_value_t<_InputIterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_InputIterator_> && std::contiguous_iterator<_OutIterator_> &&
			vectorizable_unary_function<_Function_, _InputIterator_> && vectorizable_projection<_Projection_, _InputIterator_> &&
			type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_InValue_)>{};
				return vx::__dispatch_sized_impl<__vectorized_unary_transform, _InValue_,
					std::ranges::unary_transform_result<_InputIterator_, _OutIterator_>>(
					__bytes, __first, __last, __result, __f, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__unary_impl(__first, __last, __result, __f, __proj));
	}

	template <class _InputIterator1_, class _Sentinel1_, class _InputIterator2_, class _Sentinel2_,
		class _OutIterator_, class _Function_, class _Projection1_, class _Projection2_>
	constexpr raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> __binary_transform_unchecked(
		_InputIterator1_ __first1, _Sentinel1_ __last1, _InputIterator2_ __first2, _Sentinel2_ __last2,
		_OutIterator_ __result, _Function_ __f,
		_Projection1_ __proj1, _Projection2_ __proj2) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());
		using _InValue1_ = std::iter_value_t<_InputIterator1_>;
		using _InValue2_ = std::iter_value_t<_InputIterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_InputIterator1_> && std::contiguous_iterator<_InputIterator2_> &&
			std::contiguous_iterator<_OutIterator_> && std::same_as<_InValue1_, _InValue2_> &&
			vectorizable_binary_function<_Function_, _InputIterator1_, _InputIterator2_> &&
			vectorizable_projection<_Projection1_, _InputIterator1_> && vectorizable_projection<_Projection2_, _InputIterator2_> &&
			type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				const auto __dist1 = algorithm::distance(__first1, __last1);
				const auto __dist2 = algorithm::distance(__first2, __last2);
				const auto __min_dist = (__dist1 < __dist2) ? __dist1 : __dist2;

				return vx::__dispatch_sized_impl<__vectorized_binary_transform, _InValue1_,
					std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_>>(
					__min_dist * sizeof(_InValue1_),
					__first1, __last1, __first2, __last2, __result, __f, __proj1, __proj2);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(
			__binary_impl(__first1, __last1, __first2, __last2, __result, __f, __proj1, __proj2));
	}

	template <class _InputIterator1_, class _Sentinel1_, class _InputIterator2_, class _Sentinel2_,
		class _OutIterator_, class _Function_, class _Projection1_, class _Projection2_, sizetype _Size_>
	constexpr raze_always_inline std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_> __binary_transform_unchecked(
		_InputIterator1_ __first1, _Sentinel1_ __last1, _InputIterator2_ __first2, _Sentinel2_ __last2,
		_OutIterator_ __result, _Function_ __f,
		_Projection1_ __proj1, _Projection2_ __proj2, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first1, __last1);
		__verify_range(__first2, __last2);

		using _TraitsType = decltype(this->traits());
		using _InValue1_ = std::iter_value_t<_InputIterator1_>;
		using _InValue2_ = std::iter_value_t<_InputIterator2_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InputIterator1_> 
			&& std::contiguous_iterator<_InputIterator2_> &&
			std::contiguous_iterator<_OutIterator_> && std::same_as<_InValue1_, _InValue2_> &&
			vectorizable_binary_function<_Function_, _InputIterator1_, _InputIterator2_> &&
			vectorizable_projection<_Projection1_, _InputIterator1_> &&
			vectorizable_projection<_Projection2_, _InputIterator2_> &&
			type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_InValue1_)>{};
				return vx::__dispatch_sized_impl<__vectorized_binary_transform, _InValue1_,
					std::ranges::binary_transform_result<_InputIterator1_, _InputIterator2_, _OutIterator_>>(
					__bytes, __first1, __last1, __first2, __last2, __result, __f, __proj1, __proj2);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(
			__binary_impl(__first1, __last1, __first2, __last2, __result, __f, __proj1, __proj2));
	}
};

constexpr inline auto transform = raze::options::function_with_traits<_Transform>;

__RAZE_ALGORITHM_NAMESPACE_END