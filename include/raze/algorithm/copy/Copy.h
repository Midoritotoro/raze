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
struct _Copy : _Traits_ {
	template <class _InIterator_, class _Sentinel_, class _OutIterator_>
	struct __impl {
		_InIterator_ _in_iterator;
		_Sentinel_ _in_sentinel;
		_OutIterator_ _out_iterator;

		constexpr explicit __impl(_InIterator_ __in_it, _Sentinel_ __in_sent, _OutIterator_ __out_it) noexcept :
			_in_iterator(__in_it), _in_sentinel(__in_sent), _out_iterator(__out_it)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_in_iterator == _in_sentinel) return true;
			*_out_iterator++ = *_in_iterator++;
			return false;
		}

		constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> result() const noexcept {
			return { _in_iterator, _out_iterator };
		}
	};

	template <class _Tag_>
	struct __vectorized_copy {
		template <class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(_InIterator_ __first, _Sentinel_ __sentinel, _OutIterator_ __result) const noexcept
		{
			for (; __first != __sentinel; ++__first, ++__result)
				*__result = *__first;
			return { __first, __result };
		}

		template <class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(sizetype __aligned_size, sizetype __tail_size,
			_InIterator_ __first, _Sentinel_ __sentinel, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			const auto __aligned_end = __bytes_pointer_offset(__in_ptr, __aligned_size);

			do {
				vx::store(__out_ptr, vx::load<_Tag_>(__in_ptr));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (__in_ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			return (*this)(__first, __sentinel, __result);
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>,
			_InIterator_ __first, _Sentinel_ __sentinel, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			auto __left = __iterations_aligned;

			do {
				vx::store(__out_ptr, vx::load<_Tag_>(__in_ptr));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			return (*this)(__first, __sentinel, __result);
		}
	};

	template <std::input_iterator _InIterator_, std::sentinel_for<_InIterator_> _Sentinel_,
		std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	operator()(_InIterator_ __first, _Sentinel_ __sent, _OutIterator_ __result) const noexcept
		requires(std::indirectly_copyable<_InIterator_, _OutIterator_>)
	{
		auto __r = __copy_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_InIterator_>(std::move(__sent)),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<_Range_>, _OutIterator_>
	operator()(_Range_&& __range, _OutIterator_ __result) const noexcept
		requires(!constexpr_sized_range<_Range_> && std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __copy_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<_Range_>, _OutIterator_>
	operator()(_Range_&& __range, _OutIterator_ __result) const noexcept
		requires(constexpr_sized_range<_Range_> && std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __copy_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

private:
	template <class _InIterator_, class _Sentinel_, class _OutIterator_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	__copy_unchecked(_InIterator_ __first, _Sentinel_ __last, _OutIterator_ __result) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;
		using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;

		if constexpr (std::contiguous_iterator<_InIterator_> && std::contiguous_iterator<_OutIterator_> &&
			std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8 &&
			(sizeof(_Value_) != 0) && ((sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0))
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_copy, _IntegerValue_,
					std::ranges::in_out_result<_InIterator_, _OutIterator_>>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result));
	}

	template <class _InIterator_, class _Sentinel_, class _OutIterator_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	__copy_unchecked(_InIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
		std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;
		using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;

		if constexpr (std::contiguous_iterator<_InIterator_> && std::contiguous_iterator<_OutIterator_> &&
			std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8 &&
			(sizeof(_Value_) != 0) && ((sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0))
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_IntegerValue_)>{};
				return vx::__dispatch_sized_impl<__vectorized_copy, _IntegerValue_,
					std::ranges::in_out_result<_InIterator_, _OutIterator_>>(
					__bytes, __first, __last, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result));
	}
};

constexpr inline auto copy = raze::options::function_with_traits<_Copy>;

__RAZE_ALGORITHM_NAMESPACE_END