#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>
#include <optional>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Extremes : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Projection_ _proj;
		_Comp_ _comp;
		std::iter_value_t<_Iterator_> _extreme_upper_value;
		std::iter_value_t<_Iterator_> _extreme_lower_value;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Comp_ __comp, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _proj(__proj), _comp(__comp),
			_extreme_upper_value(*__it), _extreme_lower_value(*__it)
		{
			++_iterator;
		}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			else {
				if (std::invoke(_comp, std::invoke(_proj, *_iterator), std::invoke(_proj, _extreme_lower_value))) _extreme_lower_value = *_iterator;
				else if (!std::invoke(_comp, std::invoke(_proj, *_iterator), std::invoke(_proj, _extreme_upper_value))) _extreme_upper_value = *_iterator;
				++_iterator;
				return false;
			}
		}

		raze_nodiscard constexpr raze_always_inline std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>> result() const noexcept {
			return { _extreme_lower_value, _extreme_upper_value };
		}
	};

	template <class _Tag_>
	struct __vectorized_extremes {
		template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
		raze_nodiscard raze_always_inline std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>> operator()(_Iterator_ __first,
			_Sentinel_ __sentinel, _Comp_ __comp, _Projection_ __proj) const noexcept
			requires(!vx::simd_type<_Tag_>)
		{
			std::iter_value_t<_Iterator_> __extreme_lower_value = *__first;
			std::iter_value_t<_Iterator_> __extreme_upper_value = *__first;

			for (; ++__first != __sentinel;) {
				if (__comp(__proj(*__first), __proj(__extreme_lower_value))) __extreme_lower_value = *__first;
				else if (!__comp(__proj(*__first), __proj(__extreme_upper_value))) __extreme_upper_value = *__first;
			}

			return { __extreme_lower_value, __extreme_upper_value };
		}

		template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
		raze_nodiscard raze_always_inline std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>> operator()(
			sizetype __aligned_size, sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel, _Comp_ __comp,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			auto __extreme_upper_vertical = vx::load<_Tag_>(__ptr);
			auto __extreme_lower_vertical = __extreme_upper_vertical;

			__advance_bytes(__ptr, sizeof(_Tag_));

			if (__ptr != __aligned_end) {
				do {
					const auto __loaded = __proj(vx::load<_Tag_>(__ptr));
					__extreme_lower_vertical = vx::select[__comp(__loaded, __proj(__extreme_lower_vertical)), __extreme_lower_vertical](__loaded);
					__extreme_upper_vertical = vx::select[__comp(__loaded, __proj(__extreme_upper_vertical)), __loaded](__extreme_upper_vertical);
					__advance_bytes(__ptr, sizeof(_Tag_));
				} while (__ptr != __aligned_end);
			}
			else {
				__extreme_upper_vertical = __proj(__extreme_upper_vertical);
				__extreme_lower_vertical = __extreme_upper_vertical;
			}

			auto __extreme_upper_value = vx::fold(__extreme_upper_vertical, [&] (const auto& __x, const auto& __y)
				raze_always_inline_lambda { return vx::select[__comp(__x, __y), __x](__y); });

			auto __extreme_lower_value = vx::fold(__extreme_lower_vertical, [&] (const auto& __x, const auto& __y) 
				raze_always_inline_lambda { return vx::select[__comp(__x, __y), __y](__x); });

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first) {
				if (__comp(__proj(*__first), __proj(__extreme_lower_value))) __extreme_lower_value = *__first;
				else if (!__comp(__proj(*__first), __proj(__extreme_upper_value))) __extreme_upper_value = *__first;
			}

			return { __extreme_lower_value, __extreme_upper_value };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_, class _Iterator_, 
			class _Sentinel_, class _Comp_, class _Projection_>
		raze_nodiscard raze_always_inline std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>> 
			operator()(std::integral_constant<sizetype, _AlignedSize_>, std::integral_constant<sizetype, _TailSize_>,
			_Iterator_ __first, _Sentinel_ __sentinel, _Comp_ __comp, _Projection_ __proj) 
			const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);
			auto __left = __iterations_aligned;

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __extreme_upper_vertical = vx::load<_Tag_>(__ptr);
			auto __extreme_lower_vertical = __extreme_upper_vertical;

			__advance_bytes(__ptr, sizeof(_Tag_));

			if (--__left) {
				do {
					const auto __loaded = __proj(vx::load<_Tag_>(__ptr));
					__extreme_lower_vertical = vx::select[__comp(__loaded, __proj(__extreme_lower_vertical)), __extreme_lower_vertical](__loaded);
					__extreme_upper_vertical = vx::select[__comp(__loaded, __proj(__extreme_upper_vertical)), __loaded](__extreme_upper_vertical);
					__advance_bytes(__ptr, sizeof(_Tag_));
				} while (--__left);
			}
			else {
				__extreme_upper_vertical = __proj(__extreme_upper_vertical);
				__extreme_lower_vertical = __extreme_upper_vertical;
			}

			auto __extreme_upper_value = vx::fold(__extreme_upper_vertical, [&] (const auto& __x, const auto& __y)
				raze_always_inline_lambda { return vx::select[__comp(__x, __y), __x](__y); });

			auto __extreme_lower_value = vx::fold(__extreme_lower_vertical, [&] (const auto& __x, const auto& __y) 
				raze_always_inline_lambda { return vx::select[__comp(__x, __y), __y](__x); });

			if constexpr (_TailSize_ > 0) {
				__seek_possibly_wrapped_iterator(__first, __ptr);

				do {
					if (__comp(__proj(*__first), __proj(__extreme_lower_value))) __extreme_lower_value = *__first;
					else if (!__comp(__proj(*__first), __proj(__extreme_upper_value))) __extreme_upper_value = *__first;
					++__first;
				} while (__first != __sentinel);
			}

			return { __extreme_lower_value, __extreme_upper_value };
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Comp_ = std::greater<>, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::optional<std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>>> operator()(_Iterator_ __first,
		_Sentinel_ __last, _Comp_ __comp = {}, _Projection_ __proj = {}) const noexcept
	{
		return __extremes_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__comp), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Comp_ = std::greater<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::optional<std::pair<std::ranges::range_value_t<_Range_>, std::ranges::range_value_t<_Range_>>> operator()(
		_Range_&& __range, _Comp_ __comp = {}, _Projection_ __proj = {}) const noexcept
		requires(!constexpr_sized_range<_Range_>)
	{
		return __extremes_unchecked(type_traits::__unchecked_begin(__range),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__comp), 
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Comp_ = std::greater<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::optional<std::pair<std::ranges::range_value_t<_Range_>, std::ranges::range_value_t<_Range_>>> operator()(_Range_&& __range,
		_Comp_ __comp = {}, _Projection_ __proj = {}) const noexcept requires(constexpr_sized_range<_Range_>)
	{
		return __extremes_unchecked(type_traits::__unchecked_begin(__range),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__proj),
			type_traits::__pass_function(__comp),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline std::optional<std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>>>
		__extremes_unchecked(_Iterator_ __first, _Sentinel_ __last, _Comp_ __comp, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;
		using _Return_ = std::pair<_Value_, _Value_>;

		if (__first == __last) return std::nullopt;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_binary_predicate<_Comp_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return std::optional<_Return_> { vx::__dispatch_sized_impl<__vectorized_extremes, _Value_, _Return_>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __comp, __proj) };
			}
		}

		return std::optional<_Return_> { options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __comp, __proj)) };
	}

	template <class _Iterator_, class _Sentinel_, class _Comp_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline std::optional<std::pair<std::iter_value_t<_Iterator_>, std::iter_value_t<_Iterator_>>>
		__extremes_unchecked(_Iterator_ __first, _Sentinel_ __last, _Comp_ __comp, _Projection_ __proj, 
			std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;
		using _Return_ = std::pair<_Value_, _Value_>;

		if (__first == __last) return std::nullopt;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& vectorizable_binary_predicate<_Comp_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return std::optional<_Return_> { vx::__dispatch_sized_impl<__vectorized_extremes,
					_Value_, _Return_>(__bytes, __first, __last, __comp, __proj) };
			}
		}

		return std::optional<_Return_> { options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __comp, __proj)) };
	}
};

constexpr inline auto extremes = raze::options::function_with_traits<_Extremes>;

__RAZE_ALGORITHM_NAMESPACE_END
