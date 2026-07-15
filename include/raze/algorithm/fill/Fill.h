#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>


__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _Fill : _Traits_ {
	template <class _Iterator_, class _Sentinel_, class _ValueType_>
	struct __impl {
		_Iterator_ _iterator;
		_Sentinel_ _sentinel;
		const _ValueType_& _value;

		constexpr explicit __impl(_Iterator_ __it, _Sentinel_ __sent, const _ValueType_& __v) noexcept :
			_iterator(__it), _sentinel(__sent), _value(__v)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_iterator == _sentinel) return true;
			*_iterator++ = _value;
			return false;
		}
	};

	template <class _Tag_>
	struct __vectorized_fill {
		template <class _Iterator_, class _Sentinel_, class _ValueType_>
		raze_always_inline void operator()(_Iterator_ __first, _Sentinel_ __sentinel, 
			const _ValueType_& __v) const noexcept
		{
			if constexpr (sizeof(_ValueType_) <= 8) {
				auto __start_address = std::to_address(__first);
				auto __end_address = std::to_address(__sentinel);

				auto* __reinterpret_start = reinterpret_cast<_ValueType_*>(__start_address);
				auto* __reinterpret_end = reinterpret_cast<const volatile _ValueType_* const>(__end_address);

				for (; __reinterpret_start != __reinterpret_end; ++__reinterpret_start)
					*__reinterpret_start = __v;
			}
			else {
				for (; __first != __sentinel; ++__first)
					*__first = __v;
			}
		}

		template <class _Iterator_, class _Sentinel_, class _ValueType_>
		raze_always_inline void operator()(sizetype __aligned_size, sizetype __tail_size,
			_Iterator_ __first, _Sentinel_ __sentinel, const _ValueType_& __v) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				vx::store(__ptr, _Tag_(__v));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);
			return (*this)(__first, __sentinel, __v);
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _Iterator_, class _Sentinel_, class _ValueType_>
		raze_always_inline void operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _Sentinel_ __sentinel,
			const _ValueType_& __v) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);

			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			auto __left = __iterations_aligned;

			do {
				vx::store(__ptr, _Tag_(__v));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __ptr);
			return (*this)(__first, __sentinel, __v);
		}
	};

	template <class _Iterator_, std::sentinel_for<_Iterator_> _Sentinel_, 
		class _Value_ = std::iter_value_t<_Iterator_>>
	constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first, 
		_Sentinel_ __last, const std::type_identity_t<_Value_>& __v) const noexcept
			requires(std::output_iterator<_Iterator_, _Value_>)
	{
		__fill_unchecked(type_traits::__ranges_unwrap_iterator<_Sentinel_>(std::move(__first)),
			type_traits::__ranges_unwrap_sentinel<_Iterator_>(__last), __v);
		return __last;
	}

	template <class _Range_, class _Value_ = std::ranges::range_value_t<_Range_>>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __r, const std::type_identity_t<_Value_>& __v) const noexcept
			requires(!constexpr_sized_range<_Range_> && std::ranges::output_range<_Range_, _Value_>)
	{
		auto __last = std::ranges::end(__r);
		__fill_unchecked(type_traits::__unchecked_begin(__r), type_traits::__unchecked_end(__r), __v);
		return __last;
	}

	template <class _Range_, class _Value_ = std::ranges::range_value_t<_Range_>>
	constexpr raze_always_inline std::ranges::borrowed_iterator_t<_Range_> operator()(
		_Range_&& __r, const std::type_identity_t<_Value_>& __v) const noexcept
			requires(constexpr_sized_range<_Range_> && std::ranges::output_range<_Range_, _Value_>)
	{
		auto __last = std::ranges::end(__r);
		__fill_unchecked(type_traits::__unchecked_begin(__r), type_traits::__unchecked_end(__r), __v,
			std::integral_constant<sizetype, __range_constexpr_size<_Range_>()>{});
		return __last;
	}
private:
	template <class _Iterator_, class _Sentinel_, class _ValueType_>
	constexpr raze_always_inline void __fill_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, const _ValueType_& __v) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (std::contiguous_iterator<_Iterator_> && std::is_trivially_copyable_v<_Value_> &&
			sizeof(_Value_) <= 8 && (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				vx::__dispatch_sized_impl<__vectorized_fill, _IntegerValue_, void>(algorithm::distance(__first, __last) * sizeof(_Value_),
					__first, __last, math::bit_cast<_IntegerValue_>(__v));
				return;
			}
		}

		options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __v));
	}

	template <class _Iterator_, class _Sentinel_, class _ValueType_, sizetype _Size_>
	constexpr raze_always_inline void __fill_unchecked(_Iterator_ __first, 
		_Sentinel_ __last, const _ValueType_& __v, std::integral_constant<sizetype, _Size_>) const noexcept
	{
		__verify_range(__first, __last);

		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_Iterator_> && std::is_trivially_copyable_v<_Value_> &&
			sizeof(_Value_) <= 8 && (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				vx::__dispatch_sized_impl<__vectorized_fill, _IntegerValue_, void>(std::integral_constant<
					sizetype, _Size_ * sizeof(_Value_)>{}, __first, __last, math::bit_cast<_IntegerValue_>(__v));
				return;
			}
		}

		options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __last, __v));
	}
};

constexpr inline auto fill = raze::options::function_with_traits<_Fill>;

__RAZE_ALGORITHM_NAMESPACE_END