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
struct _Replace_copy_if : _Traits_ {
	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Predicate_, class _Projection_, class _ValueType_>
	struct __impl {
		_InputIterator_ _in_iterator;
		_Sentinel_ _in_sentinel;
		_OutIterator_ _out_iterator;
		_Predicate_ _predicate;
		_Projection_ _proj;
		const _ValueType_& _new_value;

		constexpr explicit __impl(_InputIterator_ __in_it, _Sentinel_ __in_sent, _OutIterator_ __out_it,
			_Predicate_ __pred, _Projection_ __proj, const _ValueType_& __new_val) noexcept :
			_in_iterator(__in_it), _in_sentinel(__in_sent), _out_iterator(__out_it),
			_predicate(__pred), _proj(__proj), _new_value(__new_val)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_in_iterator == _in_sentinel) return true;
			*_out_iterator = _predicate(_proj(*_in_iterator)) ? _new_value : *_in_iterator;
			++_in_iterator;
			++_out_iterator;
			return false;
		}

		constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> result() const noexcept {
			return { _in_iterator, _out_iterator };
		}
	};

	template <class _Tag_>
	struct __vectorized_replace_copy {
		template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Predicate_, class _Projection_, class _ValueType_>
		raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
			_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
			_Predicate_ __predicate, _Projection_ __proj, const _ValueType_& __new_value) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __last; ++__first, ++__result)
				*__result = __predicate(__proj(*__first)) ? __new_value : *__first;

			return { std::move(__first), std::move(__result) };
		}

		template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Predicate_, class _Projection_, class _ValueType_>
		raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
			sizetype __aligned_size, sizetype __tail_size,
			_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
			_Predicate_ __predicate, _Projection_ __proj, const _ValueType_& __new_value) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			raze_assume(__in_ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__in_ptr, __aligned_size);

			do {
				auto __loaded = vx::load<_Tag_>(__in_ptr);
				vx::store(__out_ptr, vx::select[__predicate(__proj(__loaded)), __loaded](_Tag_(__new_value)));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (__in_ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			for (; __first != __last; ++__first, ++__result)
				*__result = __predicate(__proj(*__first)) ? __new_value : *__first;

			return { std::move(__first), std::move(__result) };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Predicate_, class _Projection_, class _ValueType_>
		raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
			std::integral_constant<sizetype, _AlignedSize_>, std::integral_constant<sizetype, _TailSize_>,
			_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
			_Predicate_ __predicate, _Projection_ __proj, const _ValueType_& __new_value) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			raze_assume(__in_ptr != nullptr);
			raze_assume(__out_ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				auto __loaded = vx::load<_Tag_>(__in_ptr);
				vx::store(__out_ptr, vx::select[__predicate(__proj(__loaded)), __loaded](_Tag_(__new_value)));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			for (; __first != __last; ++__first, ++__result)
				*__result = __predicate(__proj(*__first)) ? __new_value : *__first;

			return { std::move(__first), std::move(__result) };
		}
	};

	template <std::input_iterator _InputIterator_, std::sentinel_for<_InputIterator_> _Sentinel_,
		std::weakly_incrementable _OutIterator_, class _Predicate_, class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		_Predicate_ __pred, const _ValueType_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::indirectly_copyable<_InputIterator_, _OutIterator_> &&
				std::indirect_unary_predicate<_Predicate_, std::projected<_InputIterator_, _Projection_>> &&
				std::indirectly_writable<_OutIterator_, const _ValueType_&>)
	{
		auto __r = __replace_copy_if_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_InputIterator_>(std::move(__last)),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__pred), __new_value, type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_,
		class _Predicate_, class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, _OutIterator_ __result, _Predicate_ __pred,
		const _ValueType_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> &&
				std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_> &&
				std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>> &&
				std::indirectly_writable<_OutIterator_, const _ValueType_&>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __replace_copy_if_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__pred), __new_value, type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_,
		class _Predicate_, class _ValueType_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, _OutIterator_ __result, _Predicate_ __pred,
		const _ValueType_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> &&
				std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_> &&
				std::indirect_unary_predicate<_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>> &&
				std::indirectly_writable<_OutIterator_, const _ValueType_&>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __replace_copy_if_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)),
			type_traits::__pass_function(__pred), __new_value, type_traits::__pass_function(__proj),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

private:
	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Predicate_, class _ValueType_, class _Projection_>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> __replace_copy_if_unchecked(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		_Predicate_ __pred, const _ValueType_& __new_value, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _InValue_ = std::iter_value_t<_InputIterator_>;

		if constexpr (std::contiguous_iterator<_InputIterator_> && std::contiguous_iterator<_OutIterator_> &&
			vectorizable_unary_predicate<_Predicate_, _InputIterator_> &&
			vectorizable_projection<_Projection_, _InputIterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_replace_copy, _InValue_,
					std::ranges::unary_transform_result<_InputIterator_, _OutIterator_>>(
					algorithm::distance(__first, __last) * sizeof(_InValue_),
					__first, __last, __result, __pred, __proj, __new_value);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result, __pred, __proj, __new_value));
	}

	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, class _Predicate_, class _ValueType_, class _Projection_, sizetype _Size_>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> __replace_copy_if_unchecked(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		_Predicate_ __pred, const _ValueType_& __new_value, _Projection_ __proj,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _InValue_ = std::iter_value_t<_InputIterator_>;

		if constexpr (std::contiguous_iterator<_InputIterator_> && std::contiguous_iterator<_OutIterator_> &&
			vectorizable_unary_predicate<_Predicate_, _InputIterator_> &&
			vectorizable_projection<_Projection_, _InputIterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_InValue_)>{};
				return vx::__dispatch_sized_impl<__vectorized_replace_copy, _InValue_,
					std::ranges::unary_transform_result<_InputIterator_, _OutIterator_>>(
					__bytes, __first, __last, __result, __pred, __proj, __new_value);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result, __pred, __proj, __new_value));
	}
};

constexpr inline auto replace_copy_if = raze::options::function_with_traits<_Replace_copy_if>;

template <class _Traits_>
struct _Replace_copy : _Traits_ {
	template <std::input_iterator _InputIterator_, std::sentinel_for<_InputIterator_> _Sentinel_,
		std::weakly_incrementable _OutIterator_, class _ValueType1_, class _ValueType2_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<_InputIterator_, _OutIterator_> operator()(
		_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		const _ValueType1_& __old_value, const _ValueType2_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::indirectly_copyable<_InputIterator_, _OutIterator_> &&
				std::indirectly_writable<_OutIterator_, const _ValueType2_&>)
	{
		return replace_copy_if(std::move(__first), std::move(__last), std::move(__result),
			algorithm::equal_to(function_return_type<_Projection_, std::iter_value_t<_InputIterator_>>(__old_value)),
			__new_value, type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_,
		class _ValueType1_, class _ValueType2_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::unary_transform_result<std::ranges::iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, _OutIterator_ __result, const _ValueType1_& __old_value,
		const _ValueType2_& __new_value, _Projection_ __proj = {}) const noexcept
			requires(std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_> &&
				std::indirectly_writable<_OutIterator_, const _ValueType2_&>)
	{
		return replace_copy_if(std::forward<_Range_>(__range), std::move(__result),
			algorithm::equal_to(function_return_type<_Projection_, std::ranges::range_value_t<_Range_>>(__old_value)),
			__new_value, type_traits::__pass_function(__proj));
	}
};

constexpr inline auto replace_copy = raze::options::function_with_traits<_Replace_copy>;

__RAZE_ALGORITHM_NAMESPACE_END