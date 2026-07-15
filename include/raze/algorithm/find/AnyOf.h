#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Any_of : _Traits_{
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Predicate_ _predicate;
		_Projection_ _proj;
		bool _result = false;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Predicate_ __pred, _Projection_ __proj) noexcept :
			_iterator(__it), _sentinel(__sent), _predicate(__pred), _proj(__proj)
		{}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			else if (_predicate(_proj(*_iterator))) return _result = true;
			else {
				++_iterator;
				return false;
			}
		}

		raze_nodiscard constexpr raze_always_inline bool result() const noexcept {
			return _result;
		}
	};

	template <class _Tag_>
	struct __vectorized_any_of {
		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline bool operator()(_Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					return true;

			return false;
		}

		template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline bool operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel, _Predicate_ __predicate,
			_Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				if (vx::any_of(__predicate(__proj(raze::vx::load<_Tag_>(__ptr))))) return true;
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			for (; __first != __sentinel; ++__first)
				if (__predicate(__proj(*__first)))
					return true;

			return false;
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline bool operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				if (vx::any_of(__predicate(__proj(raze::vx::load<_Tag_>(__ptr))))) return true;
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			if constexpr (_TailSize_ != 0) {
				do {
					if (__predicate(__proj(*__first)))
						return true;

					++__first;
				} while (__first != __sentinel);
			}

			return false;
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Predicate_, class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline bool operator()(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		return __any_of_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)),
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(
		_Range_&& __range, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(!constexpr_sized_range<_Range_>&& std::indirect_unary_predicate<
			_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __any_of_unchecked(type_traits::__unchecked_begin(__range),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj));
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline bool operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
		requires(constexpr_sized_range<_Range_>&& std::indirect_unary_predicate<
			_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		return __any_of_unchecked(type_traits::__unchecked_begin(__range),
			type_traits::__unchecked_end(__range), type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline bool __any_of_unchecked(
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
				return vx::__dispatch_sized_impl<__vectorized_any_of, _Value_, bool>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __pred, __proj);
			}
		}

		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first, __last, __pred, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline bool __any_of_unchecked(_Iterator_ __first,
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
				return vx::__dispatch_sized_impl<__vectorized_any_of,
					_Value_, bool>(__bytes, __first, __last, __pred, __proj);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __pred, __proj));
	}
};

constexpr inline auto any_of = raze::options::function_with_traits<_Any_of>;

__RAZE_ALGORITHM_NAMESPACE_END
