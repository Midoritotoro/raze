#pragma once 

#include <raze/algorithm/order/Reverse.h>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Rotate : _Traits_ {
	template <class _Tag_>
	struct __vectorized_rotate {
		template <class _Iterator_, class _Sentinel_>
		raze_nodiscard raze_always_inline std::ranges::subrange<_Iterator_> operator()(_Iterator_ __first,
			_Iterator_ __middle, _Sentinel_ __sentinel) const noexcept
		{
			if constexpr (std::bidirectional_iterator<_Iterator_>) {
				algorithm::reverse[options::scalar](__first, __middle);
				auto __last = std::ranges::next(__first, __sentinel);
				algorithm::reverse[options::scalar](__middle, __last);

				if constexpr (std::random_access_iterator<_Iterator_>) {
					algorithm::reverse[options::scalar](__first, __last);
					return { __first + (__last - __middle), std::move(__last) };
				}
				else {
					auto __mid_last = __last;

					do {
						std::ranges::iter_swap(__first, --__mid_last);
					} while (++__first != __middle && __mid_last != __middle);

					algorithm::reverse[options::scalar](__first, __mid_last);

					if (__first == __middle) return { std::move(__mid_last), std::move(__last) };
					else return { std::move(__first), std::move(__last) };
				}
			}
			else { 
				auto __next = __middle;

				do {
					std::ranges::iter_swap(__first, __next++);
					if (++__first == __middle) __middle = __next;
				} while (__next != __sentinel);

				auto __new_first = __first;

				while (__middle != __sentinel) {
					__next = __middle;
					do {
						std::ranges::iter_swap(__first, __next++);
						if (++__first == __middle) __middle = __next;
					} while (__next != __sentinel);
				}

				return { std::move(__new_first), std::move(__middle) };
			}
		}

		template <class _Iterator_, class _Sentinel_>
		raze_nodiscard raze_always_inline std::ranges::subrange<_Iterator_> operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Iterator_ __middle, _Sentinel_ __sentinel) const noexcept requires(vx::simd_type<_Tag_>)
		{
			using _Value_ = typename std::iter_value_t<_Iterator_>;

			auto __simd_reverse_range = [&] (auto* __start, sizetype __size_bytes) raze_always_inline_lambda {
				if (raze_unlikely(__size_bytes == 0)) return;

				auto* __front = __start;
				auto* __back = __bytes_pointer_offset(__start, __size_bytes);

				const auto __iterations = (__size_bytes / sizeof(_Tag_)) >> 1;

				for (auto __i = 0; __i < __iterations; ++__i) {
					__rewind_bytes(__back, sizeof(_Tag_));

					const auto __vf = vx::load<_Tag_>(__front);
					const auto __vb = vx::load<_Tag_>(__back);

					vx::store(__front, vx::reverse(__vb));
					vx::store(__back, vx::reverse(__vf));

					__advance_bytes(__front, sizeof(_Tag_));
				}

				while (__front < __back) std::ranges::iter_swap(__front++, --__back);
			};

			auto* __ptr_first = std::to_address(__first);
			auto* __ptr_middle = std::to_address(__middle);
			auto* __ptr_last = std::to_address(__sentinel);

			const auto __left_bytes = __byte_length(__ptr_first, __ptr_middle);
			const auto __right_bytes = __byte_length(__ptr_middle, __ptr_last);

			__simd_reverse_range(__ptr_first, __left_bytes);
			__simd_reverse_range(__ptr_middle, __right_bytes);
			__simd_reverse_range(__ptr_first, __left_bytes + __right_bytes);

			__seek_possibly_wrapped_iterator(__first, __ptr_first);
			__seek_possibly_wrapped_iterator(__middle, __ptr_middle);
			__seek_possibly_wrapped_iterator(__sentinel, __ptr_last);

			return std::ranges::subrange(__first + (__sentinel - __middle), __sentinel);
		}
	};

	template <std::permutable _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_>
	constexpr raze_always_inline std::ranges::subrange<_Iterator_> operator()(
		_Iterator_ __first, _Iterator_ __middle, _Sentinel_ __sent) const noexcept
	{
		auto __r = __rotate_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__middle)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(__sent));

		__seek_possibly_wrapped_iterator(__first, __r.begin());
		__seek_possibly_wrapped_iterator(__sent, __r.end());

		return { std::move(__first), std::move(__sent) };
	}

	template <std::ranges::forward_range _Range_>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(_Range_&& __range, 
		std::ranges::iterator_t<_Range_> __middle) const noexcept
		requires(!constexpr_sized_range<_Range_> && std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		return (*this)(std::ranges::begin(__range), __middle, std::ranges::end(__range));
	}

	template <std::ranges::forward_range _Range_>
	constexpr raze_always_inline std::ranges::borrowed_subrange_t<_Range_> operator()(_Range_&& __range,
		std::ranges::iterator_t<_Range_> __middle) const noexcept
		requires(constexpr_sized_range<_Range_> && std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		auto __r = __rotate_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)),
			type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__middle)),
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(__end),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		__seek_possibly_wrapped_iterator(__begin, __r.begin());
		__seek_possibly_wrapped_iterator(__end, __r.end());

		return { std::move(__begin), std::move(__end) };
	}
private:
	template <class _Iterator_, class _Sentinel_>
	raze_nodiscard constexpr raze_always_inline std::ranges::subrange<_Iterator_> __rotate_unchecked(_Iterator_ __first,
		_Iterator_ __middle, _Sentinel_ __last) const noexcept 
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if (__first == __middle) { auto __last_it = std::ranges::next(__first, __last); return { __last_it, __last_it }; }
		if (__middle == __last) return { std::move(__first), std::move(__middle) };

		//if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
		//	&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8
		//	&& (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0) 
		//{
		//	if not consteval {
		//		using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
		//		return vx::__dispatch_sized_impl<__vectorized_rotate, _IntegerValue_, std::ranges::subrange<_Iterator_>>(
		//			algorithm::distance(__first, __last) * sizeof(_Value_), __first, __middle, __last);
		//	}
		//}

		return __vectorized_rotate<vx::scalar_tag>()(__first, __middle, __last);
	}

	template <class _Iterator_, class _Sentinel_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline std::ranges::subrange<_Iterator_> __rotate_unchecked(_Iterator_ __first,
		_Iterator_ __middle, _Sentinel_ __last, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if (__first == __middle) { auto __last_it = std::ranges::next(__first, __last); return { __last_it, __last_it }; }
		if (__middle == __last) return { std::move(__first), std::move(__middle) };

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_>
			&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8
			&& (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				return vx::__dispatch_sized_impl<__vectorized_rotate, _IntegerValue_, std::ranges::subrange<_Iterator_>>(
					std::integral_constant<sizetype, _Size_ * sizeof(_Value_)>{}, __first, __middle, __last);
			}
		}

		return __vectorized_rotate<vx::scalar_tag>()(__first, __middle, __last);
	}
};

constexpr inline auto rotate = raze::options::function_with_traits<_Rotate>;

__RAZE_ALGORITHM_NAMESPACE_END