#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Replace_if : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, class _ValueType_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		const _ValueType_& _new_value;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred,
			_Projection_ __proj, const _ValueType_& __new_val) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj), _new_value(__new_val)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			if (_predicate(_proj(*_iterator))) *_iterator = _new_value;
			++_iterator;
			return false;
		}
	};

	template <class _Tag_>
	struct __vectorized_replace {
		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, class _ValueType_>
		raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj, const _ValueType_& __new_value) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					*__first = __new_value;
		}

		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, class _ValueType_>
		raze_always_inline void operator()(sizetype __aligned_size, sizetype __tail_size,
			_Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj, const _ValueType_& __new_value) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				vx::store[__predicate(__proj(vx::load<_Tag_>(__ptr)))](__ptr, _Tag_(__new_value));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					*__first = __new_value;
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, class _ValueType_>
		raze_always_inline void operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj, const _ValueType_& __new_value) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				vx::store[__predicate(__proj(vx::load<_Tag_>(__ptr)))](__ptr, _Tag_(__new_value));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					*__first = __new_value;
		}
	};

	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __last,
		_Predicate_ __pred, const _ValueType_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		__replace_if_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__pred), __new_value, type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _ValueType_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __range, _Predicate_ __pred,
		const _ValueType_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>
				&& std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		__replace_if_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::ranges::begin(__range)),
			type_traits::__unchecked_end(__range),
			type_traits::__pass_function(__pred), __new_value, type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _ValueType_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __range, _Predicate_ __pred,
		const _ValueType_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>
				&& std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		__replace_if_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::ranges::begin(__range)),
			type_traits::__unchecked_end(__range),
			type_traits::__pass_function(__pred), __new_value, type_traits::__pass_function(__proj),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}

private:
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _ValueType_, class _Projection_>
	constexpr raze_always_inline void __replace_if_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Predicate_ __pred,
		const _ValueType_& __new_value, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				vx::__dispatch_sized_impl<__vectorized_replace, _Value_, void>(
					algorithm::distance(__first, __last) * sizeof(_Value_),
					__first, __last, __pred, __proj, __new_value);
				return;
			}
		}

		options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __pred, __proj, __new_value));
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _ValueType_, class _Projection_, sizetype _Size_>
	constexpr raze_always_inline void __replace_if_unchecked(_Iterator_ __first, _Sentinel_ __last,
		_Predicate_ __pred, const _ValueType_& __new_value, _Projection_ __proj,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
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
				vx::__dispatch_sized_impl<__vectorized_replace, _Value_, void>(
					__bytes, __first, __last, __pred, __proj, __new_value);
				return;
			}
		}

		options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __pred, __proj, __new_value));
	}
};

constexpr inline auto replace_if = raze::options::function_with_traits<_Replace_if>;

template <class _Traits_>
struct _Replace : _Traits_ {
	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _ValueType1_, class _ValueType2_, class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __last,
		const _ValueType1_& __old_value, const _ValueType2_& __new_value, _Projection_ __proj = {}) const noexcept
	{
		replace_if[_Traits_::traits()](std::move(__first), std::move(__last), algorithm::equal_to(
			function_return_type<_Projection_, std::iter_value_t<_Iterator_>>(__old_value)),
			__new_value, type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _ValueType1_, class _ValueType2_,
		class _Projection_ = std::identity>
	constexpr raze_always_inline void operator()(_Range_&& __range, const _ValueType1_& __old_value,
		const _ValueType2_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		replace_if[_Traits_::traits()](std::forward<_Range_>(__range), algorithm::equal_to(
			function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__old_value)),
			__new_value, type_traits::__pass_function(__proj));
	}
};

constexpr inline auto replace = raze::options::function_with_traits<_Replace>;

__RAZE_ALGORITHM_NAMESPACE_END