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
struct _Count_if : _Traits_ {
	template <class _DiffType_, class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		_DiffType_ _count = 0;

		constexpr explicit __impl(options::as<_DiffType_>, _Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			_count += _predicate(_proj(*_iterator));
			++_iterator;
			return false;
		}

		raze_nodiscard constexpr raze_always_inline _DiffType_ result() const noexcept {
			return _count;
		}
	};

	template <class _Tag_>
	struct __vectorized_count {
		template <class _DiffType_, class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _DiffType_ operator()(options::as<_DiffType_>, _Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			_DiffType_ __count = 0;

			for (; __first != __sentinel; ++__first)
				__count += __predicate(__proj(*__first));

			return __count;
		}

		template <class _DiffType_, class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _DiffType_ operator()(sizetype __aligned_size,
			sizetype __tail_size, options::as<_DiffType_>, _Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);
			_DiffType_ __count = 0;

			do {
				__count += raze::vx::count_set(__predicate(__proj(raze::vx::load<_Tag_>(__ptr))));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				__count += __predicate(__proj(*__first));

			return __count;
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_, class _DiffType_,
			class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _DiffType_ operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, options::as<_DiffType_>, _Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;
			_DiffType_ __count = 0;

			do {
				__count += raze::vx::count_set(__predicate(__proj(raze::vx::load<_Tag_>(__ptr))));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			if constexpr (_TailSize_ != 0) {
				do {
					__count += __predicate(__proj(*__first));
					++__first;
				} while (__first != __sentinel);
			}

			return __count;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::iter_difference_t<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return __count_unchecked<std::iter_difference_t<_Iterator_>>(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __count_unchecked<std::ranges::range_difference_t<_Range_>>(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(std::ranges::begin(__range))),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __count_unchecked<std::ranges::range_difference_t<_Range_>>(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(std::ranges::begin(__range))),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _DiffType_, class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline auto __count_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);
		
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_count, _Value_, _DiffType_>(
					algorithm::distance(__first, __last) * sizeof(_Value_), options::as<_DiffType_>{}, __first, __last, __pred, __proj);
			}
		}
		
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(options::as<_DiffType_>{}, __first, __last, __pred, __proj));
	}

	template <class _DiffType_, class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline auto __count_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_count, _Value_, _DiffType_>(__bytes, options::as<_DiffType_>{}, __first, __last, __pred, __proj);
			}
		}
		
		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(options::as<_DiffType_>{}, __first, __last, __pred, __proj));
	}
};

constexpr inline auto count_if = raze::options::function_with_traits<_Count_if>;

template <class _Traits_>
struct _Count : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::iter_difference_t<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return count_if(std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<_Range_> operator()(
		_Range_&& __range, const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		return count_if(std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<_Range_> operator()(_Range_&& __range,
		const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		return count_if(std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}
};

constexpr inline auto count = raze::options::function_with_traits<_Count>;

template <class _Traits_>
struct _Count_if_not : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::iter_difference_t<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return count_if(std::move(__first), std::move(__last), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return count_if(std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::range_difference_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return count_if(std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}
};

constexpr inline auto count_if_not = raze::options::function_with_traits<_Count_if_not>;


__RAZE_ALGORITHM_NAMESPACE_END
