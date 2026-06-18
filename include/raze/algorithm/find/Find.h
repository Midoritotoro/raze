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

		template <vx::simd_type _Tag_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, sizetype __aligned_size, sizetype __tail_size) noexcept {
			auto* __ptr = std::to_address(_iterator);
			raze_assume(__ptr != __nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				const auto __mask = _predicate(_proj(raze::vx::load<_Tag_>(__ptr)));

				if (raze::vx::any_of(__mask)) {
					__seek_possibly_wrapped_iterator(_iterator, __ptr + raze::vx::find_first_set(__mask));
					return;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(_iterator, __ptr);

			for (; _iterator != _sentinel; ++_iterator)
				if (_predicate(_proj(*_iterator)))
					break;
		}

		template <vx::simd_type _Tag_, sizetype _AlignedSize_, sizetype _TailSize_>
		raze_nodiscard raze_always_inline void operator()(_Tag_, 
			std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>) noexcept 
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(_iterator);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				const auto __mask = _predicate(_proj(raze::vx::load<_Tag_>(__ptr)));

				if (raze::vx::any_of(__mask)) {
					__seek_possibly_wrapped_iterator(_iterator, __ptr + raze::vx::find_first_set(__mask));
					return;
				}

				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);
			
			__seek_possibly_wrapped_iterator(_iterator, __ptr);

			if constexpr (_TailSize_ != 0) {
				do {
					if (_predicate(_proj(*_iterator))) break;
					++_iterator;
				} while (_iterator != _sentinel);
			}
		}

		raze_nodiscard constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
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
		using _Value_ = std::iter_value_t<_Iterator_>;

		__verify_range(__first, __last);
		auto __work = __impl(__first, __last, __pred, __proj);
		
		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				__seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl,
					_Value_, _Iterator_>(algorithm::distance(__first, __last) * sizeof(_Value_), __work));
				return __first;
			}
		}
		
		__seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work));
		return __first;
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __find_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		using _Value_ = std::iter_value_t<_Iterator_>;

		__verify_range(__first, __last);
		auto __work = __impl(__first, __last, __pred, __proj);

		if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				__seek_possibly_wrapped_iterator(__first, vx::__dispatch_sized_impl<typename options::_Unroller<decltype(this->traits())>::__impl,
					_Value_, _Iterator_>(std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{}, __work));
				return __first;
			}
		}
		
		__seek_possibly_wrapped_iterator(__first, options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work));
		return __first;
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
		return find_if(std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		return find_if(std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__v)),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Value_, 
		class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		const _Value_& __v, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		return find_if(std::forward<_Range_>(__range), algorithm::equal_to(
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
		return find_if(std::move(__first), std::move(__last), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_if(std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return find_if(std::forward<_Range_>(__range), make_not_fn(__pred), type_traits::__pass_function(__proj));
	}
};

constexpr inline auto find_if_not = raze::options::function_with_traits<_Find_if_not>;


__RAZE_ALGORITHM_NAMESPACE_END
