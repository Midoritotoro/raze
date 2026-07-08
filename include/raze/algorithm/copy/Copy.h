#pragma once 

#include <raze/concurrency/Execution.h>
#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>
#include <src/raze/algorithm/memory/Memcpy.h>

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

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_> 
			&& std::contiguous_iterator<_OutIterator_> && std::is_trivially_copyable_v<_Value_>)
		{
			if not consteval {
				auto* __first_ptr = std::to_address(__first);
				auto* __r_ptr = std::to_address(__result);

				const auto __size = __byte_length(__first_ptr, std::to_address(__last));
				auto __e = __raze_memcpy(__r_ptr, __first_ptr, __size);

				__seek_possibly_wrapped_iterator(__first, __bytes_pointer_offset(__first_ptr, __size));
				__seek_possibly_wrapped_iterator(__result, static_cast<decltype(__r_ptr)>(__e));

				return std::ranges::in_out_result(__first, __result);
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

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_>
			&& std::contiguous_iterator<_OutIterator_> && std::is_trivially_copyable_v<_Value_>)
		{
			if not consteval {
				auto* __first_ptr = std::to_address(__first);
				auto* __r_ptr = std::to_address(__result);
				auto __e = __raze_memcpy(__r_ptr, __first_ptr, std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{});

				__seek_possibly_wrapped_iterator(__first, __bytes_pointer_offset(__first_ptr, _Size_));
				__seek_possibly_wrapped_iterator(__result, static_cast<decltype(__r_ptr)>(__e));

				return std::ranges::in_out_result(__first, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __result));
	}
};

static inline constexpr auto copy = raze::options::function_with_traits<_Copy>;

__RAZE_ALGORITHM_NAMESPACE_END