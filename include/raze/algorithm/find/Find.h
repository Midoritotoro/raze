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
struct _Find_if : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			else if (_predicate(_proj(*_iterator))) return true;
			else {
				++_iterator;
				return false;
			}
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <class _Tag_>
	struct __vectorized_find {
		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _Iterator_ operator()(_Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					break;

			return __first;
		}

		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _Iterator_ operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				if (const auto __mask = __predicate(__proj(raze::vx::load<_Tag_>(__ptr))); raze::vx::any_of(__mask)) {
					__seek_possibly_wrapped_iterator(__first, __ptr + raze::vx::find_first_set[vx::not_null](__mask));
					return __first;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					break;

			return __first;
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_, 
			class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _Iterator_ operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				if (const auto __mask = __predicate(__proj(raze::vx::load<_Tag_>(__ptr))); raze::vx::any_of(__mask)) {
					__seek_possibly_wrapped_iterator(__first, __ptr + raze::vx::find_first_set[vx::not_null](__mask));
					return __first;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			if constexpr (_TailSize_ != 0) {
				do {
					if (__predicate(__proj(*__first)))
						break;

					++__first;
				} while (__first != __sentinel);
			}

			return __first;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj)));

		return __first;
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		auto __first = std::ranges::begin(__range);
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__first)), 
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj)));
		return __first;
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		auto __first = std::ranges::begin(__range);
		__seek_possibly_wrapped_iterator(__first, __find_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__first)),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{}));
		return __first;
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);
		
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_find, _Value_, _Iterator_>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __pred, __proj);
			}
		}
		
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first, __last, __pred, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& vectorizable_unary_predicate<_Predicate_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_find,
					_Value_, _Iterator_>(__bytes, __first, __last, __pred, __proj);
			}
		}
		
		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __pred, __proj));
	}
};

constexpr inline auto find_if = raze::options::function_with_traits<_Find_if>;

template <class _Traits_>
struct _Find : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, const _Value_& __v, _Projection_ __proj = {}) const noexcept
	{
		return find_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		return find_if[_Traits_::traits()](std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_, 
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		return find_if[_Traits_::traits()](std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}
};

constexpr inline auto find = raze::options::function_with_traits<_Find>;

template <class _Traits_>
struct _Find_if_not : _Traits_ {
	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return find_if[_Traits_::traits()](std::move(__first), std::move(__last), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_if[_Traits_::traits()](std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_if[_Traits_::traits()](std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}
};

constexpr inline auto find_if_not = raze::options::function_with_traits<_Find_if_not>;


__RAZE_ALGORITHM_NAMESPACE_END
