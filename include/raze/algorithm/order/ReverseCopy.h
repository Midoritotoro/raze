#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Reverse_copy : _Traits_ {
	template <class _InIterator_, class _Sentinel_, class _OutIterator_>
	struct __impl {
		_InIterator_ _current;
		_InIterator_ _begin;
		_InIterator_ _end;
		_OutIterator_ _out_it;

		constexpr explicit __impl(_InIterator_ __first, _Sentinel_ __last, _OutIterator_ __out_it) noexcept {
			_begin = __first;
			_end = std::ranges::next(__first, __last);
			_current = _end;
			_out_it = __out_it;
		}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_current == _begin) return true;
			--_current;
			*_out_it++ = *_current;
			return false;
		}

		raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> result() const noexcept {
			return { _end, _out_it };
		}
	};

	template <class _Tag_>
	struct __vectorized_reverse_copy {
		template <class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_nodiscard raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(_InIterator_ __first, _Sentinel_ __sentinel, _OutIterator_ __result) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			auto __end_it = std::ranges::next(__first, __sentinel);
			auto __it = __end_it;
			while (__it != __first) {
				--__it;
				*__result++ = *__it;
			}
			return { __end_it, __result };
		}

		template <class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_nodiscard raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(sizetype __aligned_size, sizetype __tail_size, _InIterator_ __first, 
			_Sentinel_ __sentinel, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __in_begin = std::to_address(__first);
			auto* __in_end = std::to_address(__sentinel);
			auto* __out_ptr = std::to_address(__result);

			if (__tail_size != 0) {
				const auto __tail_count = __tail_size / sizeof(typename _Tag_::value_type);
				for (auto __i = 0; __i < __tail_count; ++__i) {
					--__in_end;
					*__out_ptr++ = *__in_end;
				}
			}

			auto __count = __aligned_size / sizeof(_Tag_);

			do {
				__rewind_bytes(__in_end, sizeof(_Tag_));
				const auto __v = vx::load<_Tag_>(__in_end);
				vx::store(__out_ptr, vx::reverse(__v));
				__advance_bytes(__out_ptr, sizeof(_Tag_));
			} while (--__count);

			__seek_possibly_wrapped_iterator(__result, __out_ptr);
			return { __sentinel, __result };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_nodiscard raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _InIterator_ __first,
			_Sentinel_ __sentinel, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __in_begin = std::to_address(__first);
			auto* __in_end = std::to_address(__sentinel);
			auto* __out_ptr = std::to_address(__result);

			if constexpr (_TailSize_ != 0) {
				constexpr auto __tail_count = _TailSize_ / sizeof(typename _Tag_::value_type);
				for (auto __i = 0; __i < __tail_count; ++__i) {
					--__in_end;
					*__out_ptr++ = *__in_end;
				}
			}

			constexpr auto __count = _AlignedSize_ / sizeof(_Tag_);
			auto __left = __count;

			do {
				__rewind_bytes(__in_end, sizeof(_Tag_));
				const auto __v = vx::load<_Tag_>(__in_end);
				vx::store(__out_ptr, vx::reverse(__v));
				__advance_bytes(__out_ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__result, __out_ptr);
			return { __sentinel, __result };
		}
	};

	template <std::bidirectional_iterator _InIterator_, std::sentinel_for<_InIterator_> _Sentinel_,
		std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	operator()(_InIterator_ __first, _Sentinel_ __sent, _OutIterator_ __result) const noexcept
		requires(std::indirectly_copyable<_InIterator_, _OutIterator_>)
	{
		auto __r = __reverse_copy_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_InIterator_>(std::move(__sent)),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

	template <std::ranges::bidirectional_range _Range_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<_Range_>, _OutIterator_>
	operator()(_Range_&& __range, _OutIterator_ __result) const noexcept
		requires(!constexpr_sized_range<_Range_> && std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __reverse_copy_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(__end),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

	template <std::ranges::bidirectional_range _Range_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<_Range_>, _OutIterator_>
	operator()(_Range_&& __range, _OutIterator_ __result) const noexcept
		requires(constexpr_sized_range<_Range_> && std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __reverse_copy_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(__end),
			algorithm::__unwrap_iterator(std::move(__result)),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

private:
	template <class _InIterator_, class _Sentinel_, class _OutIterator_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	__reverse_copy_unchecked(_InIterator_ __first, _Sentinel_ __last, _OutIterator_ __result) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_>
			&& std::contiguous_iterator<_OutIterator_> && std::is_trivially_copyable_v<_Value_> 
			&& sizeof(_Value_) <= 8 && (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = typename IntegerForSizeof<_Value_>::Unsigned;
				return vx::__dispatch_sized_impl<__vectorized_reverse_copy, _IntegerValue_,
					std::ranges::in_out_result<_InIterator_, _OutIterator_>>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result));
	}

	template <class _InIterator_, class _Sentinel_, class _OutIterator_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	__reverse_copy_unchecked(_InIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_> && 
			std::contiguous_iterator<_OutIterator_> && std::is_trivially_copyable_v<_Value_> &&
			sizeof(_Value_) <= 8 && (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = typename IntegerForSizeof<_Value_>::Unsigned;
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_IntegerValue_)>{};
				return vx::__dispatch_sized_impl<__vectorized_reverse_copy, _IntegerValue_,
					std::ranges::in_out_result<_InIterator_, _OutIterator_>>(
					__bytes, __first, __last, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result));
	}
};

constexpr inline auto reverse_copy = raze::options::function_with_traits<_Reverse_copy>;

__RAZE_ALGORITHM_NAMESPACE_END