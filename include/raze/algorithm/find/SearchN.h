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
struct _Search_n : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _Size_,
		class _ValueType_, class _Predicate_, class _Projection_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		_Size_ _count;
		_Size_ _current_count;
		_ValueType_ _value;
		_Predicate_ _predicate;
		_Projection_ _proj;
		_Iterator_ _candidate;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, _Size_ __count,
			const _ValueType_& __value, _Predicate_ __pred, _Projection_ __proj) noexcept: 
			_iterator(__it), _sentinel(__sent), _count(__count), _current_count(0),
			_value(__value), _predicate(__pred), _proj(__proj), _candidate(__it)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_count <= 0) return true;
			if (_iterator == _sentinel) return true;

			if (_predicate(_proj(*_iterator), _value)) {
				if (_current_count == 0) _candidate = _iterator;

				++_current_count;
				++_iterator;

				return _current_count >= _count;
			}

			_current_count = 0;
			++_iterator;
			return false;
		}

		raze_always_inline constexpr std::ranges::subrange<_Iterator_> result() const noexcept {
			if (_count <= 0) return std::ranges::subrange(_candidate, _candidate);
			if (_current_count >= _count) return std::ranges::subrange(_candidate, _iterator);

			return std::ranges::subrange(_iterator, _iterator);
		}
	};

	template <class _Tag_>
	struct __vectorized_search_n {
		template <class _Iterator_, class _Sentinel_, class _Value_, 
			class _SizeType_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first, 
			_Sentinel_ __sentinel, _SizeType_ __count, const _Value_& __v, _Predicate_ __predicate, 
			_Projection_ __proj) const noexcept
		{
			if (__count <= 0)
				return { __first, __first };

			for (; __first != __sentinel; ++__first) {
				if (__predicate(__proj(*__first), __v)) {
					auto __start = __first;
					auto __n = _SizeType_(1);

					for (;;) {
						if (__n++ == __count) return { __start, std::ranges::next(__first) };
						if (++__first == __sentinel) return { __first, __first };
						if (!__predicate(__proj(*__first), __v)) break;
					}
				}
			}

			return { __first, __first };
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
			
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);

			
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_, 
			class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_>
		raze_nodiscard raze_always_inline _Iterator_ operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _Sentinel_ __sentinel,
			_Predicate_ __predicate, _Projection_ __proj) const noexcept requires(vx::simd_type<_Tag_>)
		{
			
		}
	};

	template <std::input_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_,
		class _Value_ = std::iter_value_t<_Iterator_>, class _Predicate_ = std::equal_to<>, 
		class _Projection_ = std::identity>
	raze_nodiscard constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first,
		_Sentinel_ __last, std::iter_difference_t<_Iterator_> __count, const std::type_identity_t<_Value_>& __v,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(std::indirect_unary_predicate<_Predicate_, std::projected<_Iterator_, _Projection_>>)
	{
		__seek_possibly_wrapped_iterator(__first, __search_n_unchecked(
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(std::move(__last)), __count, __v,
			type_traits::__pass_function(__pred), type_traits::__pass_function(__proj)));

		return __first;
	}

	template <std::ranges::input_range _Range_, class _Value_ = std::ranges::range_value_t<_Range_>,
		class _Predicate_ = std::equal_to<>, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(
		_Range_&& __range, std::ranges::range_difference_t<_Range_> __count, 
		const std::type_identity_t<_Value_>& __v, _Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		auto __first = std::ranges::begin(__range);
		__seek_possibly_wrapped_iterator(__first, __search_n_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__first)), 
			type_traits::__unchecked_end(__range), __count, __v, type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj)));
		return __first;
	}

	template <std::ranges::input_range _Range_, class _Predicate_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(_Range_&& __range,
		_Predicate_ __pred, _Projection_ __proj = {}) const noexcept
			requires(constexpr_sized_range<_Range_> && std::indirect_unary_predicate<
				_Predicate_, std::projected<std::ranges::iterator_t<_Range_>, _Projection_>>)
	{
		
		auto __first = std::ranges::begin(__range);
		__seek_possibly_wrapped_iterator(__first, __search_n_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__first)),
			type_traits::__unchecked_end(__range), __count, __v, type_traits::__pass_function(__pred),
			type_traits::__pass_function(__proj), std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{}));
		return __first;
	}
private:
	template <class _Iterator_, class _Sentinel_, class _Value_, 
		class _SizeType_, class _Predicate_, class _Projection_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __search_n_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, const _Value_& __v, _SizeType_ __size, _Predicate_ __pred, 
		_Projection_ __proj) const noexcept
	{
		__verify_range(__first, __last);
		
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		/*if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_find, _Value_, _Iterator_>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __pred, __proj);
			}
		}*/
		
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__impl(__first, __last, __pred, __proj));
	}

	template <class _Iterator_, class _Sentinel_, class _Predicate_, class _Projection_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline _Iterator_ __search_n_unchecked(_Iterator_ __first,
		_Sentinel_ __last, _Predicate_ __pred, _Projection_ __proj, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		/*if constexpr (std::contiguous_iterator<_Iterator_> && vectorizable_unary_predicate<_Predicate_, _Iterator_>
			&& vectorizable_projection<_Projection_, _Iterator_>)
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{};
				return vx::__dispatch_sized_impl<__vectorized_find,
					_Value_, _Iterator_>(__bytes, __first, __last, __pred, __proj);
			}
		}*/
		
		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __pred, __proj));
	}
};

constexpr inline auto search_n = raze::options::function_with_traits<_Search_n>;

__RAZE_ALGORITHM_NAMESPACE_END
