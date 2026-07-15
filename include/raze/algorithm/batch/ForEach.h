#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _For_each : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Function_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Function_ _function;
		_Projection_ _proj;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Function_ __f, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _function(__f), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			_function(_proj(*_iterator++));
			return false;
		}

		constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> result() const noexcept {
			return { _iterator, _function };
		}
	};

	template <class _Tag_>
	struct __vectorized_for_each {
		template <class _Iterator_, class _Sentinel_, class _Function_, class _Projection_>
		raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> operator()(_Iterator_ __first, _Sentinel_ __sentinel, _Function_ __f,
			_Projection_ __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __sentinel; ++__first)
				__f(__proj(*__first));

			return { std::move(__first), std::move(__f) };
		}

		template <class _Iterator_, class _Sentinel_, class _Function_, class _Projection_>
		raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel, _Function_ __f,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				auto __projected = __proj(raze::vx::load<_Tag_>(__ptr));
				__f(__projected);
				vx::store(__ptr, __projected);
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				__f(__proj(*__first));

			return { std::move(__first), std::move(__f) };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_, 
			class _Iterator_, class _Sentinel_, class _Function_, class _Projection_>
		raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _Sentinel_ __sentinel,
			_Function_ __f, _Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				auto __projected = __proj(raze::vx::load<_Tag_>(__ptr));
				__f(__projected);
				vx::store(__ptr, __projected);
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				__f(__proj(*__first));

			return { std::move(__first), std::move(__f) };
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> operator()(_Iterator_ __first,
		_Sentinel_ __last, _Function_ __f, _Projection_ __proj = {}) const noexcept
	{
		auto __r = __for_each_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__f), type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__first, __r.in);
		return { __first, __unwrap_function(std::move(__r.fun)) };
	}

	template <std::ranges::input_range _Range_, class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::for_each_result<std::ranges::iterator_t<_Range_>, _Function_> operator()(
		_Range_&& __range, _Function_ __f, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __r = __for_each_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__f),
			type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__begin, __r.in);
		return { __begin, __unwrap_function(std::move(__r.fun)) };
	}

	template <std::ranges::input_range _Range_, class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::for_each_result<std::ranges::iterator_t<_Range_>, _Function_> operator()(_Range_&& __range,
		_Function_ __f, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __r = __for_each_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__f),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, __r.in);
		return { __begin, __unwrap_function(std::move(__r.fun)) };
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Function_, class _Projection_>
	constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> __for_each_unchecked(
		_Iterator_ __first, _Sentinel_ __last, _Function_ __f, _Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);
		
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& vectorizable_unary_function<_Function_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_> && type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_for_each, _Value_, std::ranges::for_each_result<_Iterator_, _Function_>>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __f, __proj);
			}
		}
		
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first, __last, __f, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Function_, class _Projection_, sizetype _Size_>
	constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> __for_each_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Function_ __f, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& vectorizable_unary_function<_Function_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_> && type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_for_each,
					_Value_, std::ranges::for_each_result<_Iterator_, _Function_>>(__bytes, __first, __last, __f, __proj);
			}
		}
		
		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __f, __proj));
	}
};

constexpr inline auto for_each = raze::options::function_with_traits<_For_each>;

__RAZE_ALGORITHM_NAMESPACE_END