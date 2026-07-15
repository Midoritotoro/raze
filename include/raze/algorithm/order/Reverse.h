#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Reverse : _Traits_ {
	template <class _Iterator_, class _Sentinel_>
	struct __impl {
		_Iterator_ _forward_it;
		_Iterator_ _backward_it;

		constexpr explicit __impl(_Iterator_ __first, _Sentinel_ __last) noexcept {
			_forward_it = __first;
			if constexpr (std::same_as<_Iterator_, _Sentinel_>) _backward_it = __last;
			else _backward_it = std::ranges::next(__first, __last);
		}

		template <class _Tag_>
		raze_always_inline raze_nodiscard constexpr bool operator()(_Tag_) noexcept {
			if (_forward_it == _backward_it) return true;

			--_backward_it;

			if (_forward_it == _backward_it) return true;

			std::ranges::iter_swap(_forward_it, _backward_it);
			++_forward_it;

			return false;
		}
	};

	template <class _Tag_>
	struct __vectorized_reverse {
		template <class _Iterator_, class _Sentinel_>
		raze_nodiscard raze_always_inline void operator()(_Iterator_ __first,
			_Sentinel_ __sentinel) const noexcept requires(!vx::simd_type<_Tag_>)
		{
			_Iterator_ __end_it = std::ranges::next(__first, __sentinel);
			for (; !(__first == __end_it || __first == --__end_it); ++__first)
				std::ranges::iter_swap(__first, __end_it);
		}

		template <class _Iterator_, class _Sentinel_>
		raze_nodiscard raze_always_inline void operator()(sizetype __aligned_size,
			sizetype __tail_size, _Iterator_ __first, _Sentinel_ __sentinel) const noexcept requires(vx::simd_type<_Tag_>)
		{
			using _Value_ = typename std::iter_value_t<_Iterator_>;
			
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			_Iterator_ __end_it = std::ranges::next(__first, __sentinel);

			const auto __size = __aligned_size + __tail_size;
			const auto __num_vecs = __aligned_size / sizeof(_Tag_);
			raze_assume(__num_vecs > 0);

			const auto __pairs = __num_vecs >> 1;

			auto* __ptr_front = __ptr;
			auto* __ptr_back = __bytes_pointer_offset(__ptr, __size);

			for (sizetype __i = 0; __i < __pairs; ++__i) {
				__rewind_bytes(__ptr_back, sizeof(_Tag_));

				const auto __vf = vx::load<_Tag_>(__ptr_front);
				const auto __vb = vx::load<_Tag_>(__ptr_back);

				vx::store(__ptr_front, vx::reverse(__vb));
				vx::store(__ptr_back, vx::reverse(__vf));

				__advance_bytes(__ptr_front, sizeof(_Tag_));
			}

			__seek_possibly_wrapped_iterator(__first, __ptr_front);
			
			_Iterator_ __back_it;
			__seek_possibly_wrapped_iterator(__back_it, __ptr_back);

			while (__first < __back_it) {
				--__back_it;
				std::ranges::iter_swap(__first, __back_it);
				++__first;
			}
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_, class _Iterator_, class _Sentinel_>
		raze_nodiscard raze_always_inline void operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first,
			_Sentinel_ __sentinel) const noexcept requires(vx::simd_type<_Tag_>)
		{
			using _Value_ = typename std::iter_value_t<_Iterator_>;
			constexpr auto __size = _AlignedSize_ + _TailSize_;
			constexpr auto __num_vecs = _AlignedSize_ / sizeof(_Tag_);
			constexpr auto __pairs = __num_vecs / 2;

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			_Iterator_ __end_it = std::ranges::next(__first, __sentinel);

			auto* __ptr_front = __ptr;
			auto* __ptr_back = __bytes_pointer_offset(__ptr, __size);

			for (sizetype __i = 0; __i < __pairs; ++__i) {
				__rewind_bytes(__ptr_back, sizeof(_Tag_));

				const auto __vf = vx::load<_Tag_>(__ptr_front);
				const auto __vb = vx::load<_Tag_>(__ptr_back);

				vx::store(__ptr_front, vx::reverse(__vb));
				vx::store(__ptr_back, vx::reverse(__vf));

				__advance_bytes(__ptr_front, sizeof(_Tag_));
			}

			__seek_possibly_wrapped_iterator(__first, __ptr_front);
			
			if constexpr (_TailSize_ > 0) {
				_Iterator_ __back_it = __first + (__ptr_back - __ptr);
					
				while (__first < __back_it) {
					--__back_it;
					std::ranges::iter_swap(__first, __back_it);
					++__first;
				}
			}
		}
	};

	template <std::bidirectional_iterator _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_>
	constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first, _Sentinel_ __sent) const noexcept
		requires(std::permutable<_Iterator_>)
	{
		__reverse_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(__sent));
		return std::ranges::next(__first, __sent);
	}

	template <std::ranges::bidirectional_range _Range_>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range) const noexcept
		requires(!constexpr_sized_range<_Range_> && std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		return (*this)(std::ranges::begin(__range), std::ranges::end(__range));
	}

	template <std::ranges::bidirectional_range _Range_>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(_Range_&& __range) const noexcept
		requires(constexpr_sized_range<_Range_> && std::permutable<std::ranges::iterator_t<_Range_>>)
	{
		auto __begin = std::ranges::begin(__range);
		auto __end = std::ranges::end(__range);

		__reverse_unchecked(type_traits::__ranges_unwrap_range_iterator<_Range_>(std::move(__begin)), 
			type_traits::__ranges_unwrap_range_sentinel<_Range_>(__end),
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});

		return std::ranges::next(__begin, __end);
	}
private:
	template <class _Iterator_, class _Sentinel_>
	raze_nodiscard constexpr raze_always_inline void __reverse_unchecked(_Iterator_ __first, _Sentinel_ __last) const noexcept {
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> &&
			std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8
			&& (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0) 
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				return vx::__dispatch_sized_impl<__vectorized_reverse, _IntegerValue_, void>(
					algorithm::distance(__first, __last) * sizeof(_Value_), __first, __last);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last));
	}

	template <class _Iterator_, class _Sentinel_, sizetype _Size_>
	raze_nodiscard constexpr raze_always_inline void __reverse_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, std::integral_constant<sizetype, _Size_> __size) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8
			&& (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				constexpr auto __bytes = std::integral_constant<sizetype, _Size_ * sizeof(_IntegerValue_)>{};
				return vx::__dispatch_sized_impl<__vectorized_reverse, _IntegerValue_, void>(__bytes, __first, __last);
			}
		}
		
		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last));
	}
};

constexpr inline auto reverse = raze::options::function_with_traits<_Reverse>;

__RAZE_ALGORITHM_NAMESPACE_END