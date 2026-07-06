#pragma once 

#include <raze/algorithm/order/Reverse.h>
#include <raze/algorithm/swap/Swap.h>
#include <raze/algorithm/copy/Copy.h>
#include <raze/algorithm/copy/CopyN.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Rotate_copy : _Traits_ {
	template <class _Tag_>
	struct __vectorized_rotate_copy {
		template <class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_nodiscard raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
			operator()(_InIterator_ __first, _InIterator_ __middle, _Sentinel_ __last, _OutIterator_ __result) const noexcept
		{
			auto __c1 = algorithm::copy[options::scalar](__middle, std::move(__last), std::move(__result));
			auto __c2 = algorithm::copy[options::scalar](std::move(__first), std::move(__middle), std::move(__c1.out));
			return { std::move(__c1.in), std::move(__c2.out) };
		}

		template <class _InIterator_, class _Sentinel_, class _OutIterator_>
		raze_nodiscard raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> operator()(
			sizetype, sizetype, _InIterator_ __first, _InIterator_ __middle, _Sentinel_ __last, _OutIterator_ __result) const noexcept
		{
			auto __c1 = algorithm::copy(__middle, std::move(__last), std::move(__result));
			auto __c2 = algorithm::copy(std::move(__first), std::move(__middle), std::move(__c1.out));
			return { std::move(__c1.in), std::move(__c2.out) };
		}
	};

	template <std::input_iterator _InIterator_, std::sentinel_for<_InIterator_> _Sentinel_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> operator()(
		_InIterator_ __first, _InIterator_ __middle, _Sentinel_ __last, _OutIterator_ __result) const noexcept
		requires(std::indirectly_copyable<_InIterator_, _OutIterator_>)
	{
		auto __r = __rotate_copy_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__middle)),
			type_traits::__ranges_unwrap_sentinel<_InIterator_>(std::move(__last)),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__first, __r.in);
		__seek_possibly_wrapped_iterator(__result, __r.out);

		return { std::move(__first), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, std::ranges::iterator_t<_Range_> __middle, _OutIterator_ __result) const noexcept
		requires(!constexpr_sized_range<_Range_> && std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __rotate_copy_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__middle)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__begin, __r.in);
		__seek_possibly_wrapped_iterator(__result, __r.out);

		return { std::move(__begin), std::move(__result) };
	}

	template <std::ranges::input_range _Range_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<std::ranges::borrowed_iterator_t<_Range_>, _OutIterator_> operator()(
		_Range_&& __range, std::ranges::iterator_t<_Range_> __middle, _OutIterator_ __result) const noexcept
		requires(constexpr_sized_range<_Range_> && std::indirectly_copyable<std::ranges::iterator_t<_Range_>, _OutIterator_>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __rotate_copy_unchecked(
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__middle)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(std::move(__end)),
			algorithm::__unwrap_iterator(std::move(__result)),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, __r.in);
		__seek_possibly_wrapped_iterator(__result, __r.out);

		return { std::move(__begin), std::move(__result) };
	}

private:
	template <class _InIterator_, class _Sentinel_, class _OutIterator_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> 
	__rotate_copy_unchecked(_InIterator_ __first, _InIterator_ __middle, _Sentinel_ __last, _OutIterator_ __result) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_> && std::contiguous_iterator<_OutIterator_>
			&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8
			&& (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0) 
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				return vx::__dispatch_sized_impl<__vectorized_rotate_copy, _IntegerValue_, std::ranges::in_out_result<_InIterator_, _OutIterator_>>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __middle, __last, __result);
			}
		}

		auto __c1 = algorithm::copy[options::scalar](__middle, std::move(__last), std::move(__result));
		auto __c2 = algorithm::copy[options::scalar](std::move(__first), std::move(__middle), std::move(__c1.out));
		return { std::move(__c1.in), std::move(__c2.out) };
	}

	template <class _InIterator_, class _Sentinel_, class _OutIterator_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> 
	__rotate_copy_unchecked(_InIterator_ __first, _InIterator_ __middle, _Sentinel_ __last, _OutIterator_ __result, 
	                        std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_> && std::contiguous_iterator<_OutIterator_>
			&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8
			&& (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				return vx::__dispatch_sized_impl<__vectorized_rotate_copy, _IntegerValue_, std::ranges::in_out_result<_InIterator_, _OutIterator_>>(
					std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{}, __first, __middle, __last, __result);
			}
		}

		auto __c1 = algorithm::copy[options::scalar](__middle, std::move(__last), std::move(__result));
		auto __c2 = algorithm::copy[options::scalar](std::move(__first), std::move(__middle), std::move(__c1.out));
		return { std::move(__c1.in), std::move(__c2.out) };
	}
};

constexpr inline auto rotate_copy = raze::options::function_with_traits<_Rotate_copy>;

__RAZE_ALGORITHM_NAMESPACE_END