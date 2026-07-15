#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Copy_backward : _Traits_ {
	template <class _InputIterator_, class _OutIterator_>
	struct __impl {
		_InputIterator_ _in_first;
		_InputIterator_ _in_last;
		_InputIterator_ _in_last_cached;
		_OutIterator_ _out_iterator;

		constexpr explicit __impl(_InputIterator_ __first, _InputIterator_ __last, _OutIterator_ __out_it) noexcept :
			_in_first(__first), _in_last_cached(__last), _in_last(__last), _out_iterator(__out_it)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_in_last == _in_first) return true;
			*--_out_iterator = *--_in_last;
			return false;
		}

		constexpr raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_> result() const noexcept {
			return { _in_last_cached, _out_iterator };
		}
	};

	template <class _Tag_>
	struct __vectorized_copy_backward {
		template <class _InputIterator_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_>
			operator()(_InputIterator_ __first, _InputIterator_ __last, _OutIterator_ __result) const noexcept
		{
			auto __cached_last = __last;
			while (__first != __last) *--__result = *--__last;
			return { __cached_last, __result };
		}

		template <class _InputIterator_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_>
			operator()(sizetype __aligned_size, sizetype __tail_size,
				_InputIterator_ __first, _InputIterator_ __last, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			using _Value_ = std::iter_value_t<_InputIterator_>;
			constexpr auto __elem_size = sizeof(_Value_);

			auto* __in_ptr = std::to_address(__last);
			auto* __out_ptr = std::to_address(__result);

			const auto __tail_elems = __tail_size / __elem_size;
			for (sizetype __i = 0; __i < __tail_elems; ++__i)
				*--__out_ptr = *--__in_ptr;

			auto* __aligned_start = std::to_address(__first);
			
			do {
				__rewind_bytes(__in_ptr, sizeof(_Tag_));
				__rewind_bytes(__out_ptr, sizeof(_Tag_));
				vx::store(__out_ptr, vx::load<_Tag_>(__in_ptr));
			} while (__in_ptr != __aligned_start);

			__seek_possibly_wrapped_iterator(__result, __out_ptr);
			return { __last, __result };
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InputIterator_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_>
			operator()(std::integral_constant<sizetype, _AlignedSize_>,
				std::integral_constant<sizetype, _TailSize_>,
				_InputIterator_ __first, _InputIterator_ __last, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			using _Value_ = std::iter_value_t<_InputIterator_>;
			constexpr auto __elem_size = sizeof(_Value_);
			constexpr auto __tail_elems = _TailSize_ / __elem_size;
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __in_ptr = std::to_address(__last);
			auto* __out_ptr = std::to_address(__result);

			if constexpr (__tail_elems > 0) {
				auto __left_tail = __tail_elems;
				do {
					*--__out_ptr = *--__in_ptr;
				} while (--__left_tail);
			}

			auto __left = __iterations_aligned;

			do {
				__rewind_bytes(__in_ptr, sizeof(_Tag_));
				__rewind_bytes(__out_ptr, sizeof(_Tag_));
				vx::store(__out_ptr, vx::load<_Tag_>(__in_ptr));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__result, __out_ptr);
			return { __last, __result };
		}
	};

	template <std::bidirectional_iterator _InputIterator_, std::sentinel_for<_InputIterator_> _Sentinel_,
		std::bidirectional_iterator _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_>
		operator()(_InputIterator_ __first, _Sentinel_ __sent, _OutIterator_ __result) const noexcept
		requires(std::indirectly_copyable<_InputIterator_, _OutIterator_>)
	{
		auto __r = __copy_backward_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_InputIterator_>(std::move(__sent)),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

	template <std::ranges::bidirectional_range _Range_, std::bidirectional_iterator _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<_Range_>, _OutIterator_>
		operator()(_Range_&& __range, _OutIterator_ __result) const noexcept
		requires(!constexpr_sized_range<_Range_>&& std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __r = __copy_backward_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__unchecked_end(__range), algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

	template <std::ranges::bidirectional_range _Range_, std::bidirectional_iterator _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::iterator_t<_Range_>, _OutIterator_>
		operator()(_Range_&& __range, _OutIterator_ __result) const noexcept
		requires(constexpr_sized_range<_Range_>&& std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);

		auto __r = __copy_backward_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__unchecked_end(__range), algorithm::__unwrap_iterator(std::move(__result)),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__begin), std::move(__result) };
	}

private:
	template <class _InputIterator_, class _Sentinel_, class _OutIterator_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_>
		__copy_backward_unchecked(_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InputIterator_>;
		using _IntegerValue_ = typename IntegerForSizeof<_Value_>::Unsigned;

		auto __last_it = std::ranges::next(__first, __last);

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_InputIterator_> && std::contiguous_iterator<_OutIterator_>
			&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8 &&
			(sizeof(_Value_) != 0) && ((sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0))
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_copy_backward, _IntegerValue_,
					std::ranges::in_out_result<_InputIterator_, _OutIterator_>>(
						algorithm::distance(__first, __last_it) * sizeof(_Value_),
						__first, __last_it, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last_it, __result));
	}

	template <class _InputIterator_, class _Sentinel_, class _OutIterator_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InputIterator_, _OutIterator_>
		__copy_backward_unchecked(_InputIterator_ __first, _Sentinel_ __last, _OutIterator_ __result,
			std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InputIterator_>;
		using _IntegerValue_ = typename IntegerForSizeof<_Value_>::Unsigned;

		auto __last_it = std::ranges::next(__first, __last);

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_InputIterator_> && std::contiguous_iterator<_OutIterator_>
			&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8 &&
			(sizeof(_Value_) != 0) && ((sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0))
		{
			if not consteval {
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_IntegerValue_)>{};
				return vx::__dispatch_sized_impl<__vectorized_copy_backward, _IntegerValue_,
					std::ranges::in_out_result<_InputIterator_, _OutIterator_>, options::__get_forced_isa<_TraitsType>()>(
						__bytes, __first, __last_it, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last_it, __result));
	}
};

constexpr inline auto copy_backward = raze::options::function_with_traits<_Copy_backward>;

__RAZE_ALGORITHM_NAMESPACE_END
