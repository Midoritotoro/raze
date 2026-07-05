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
struct _Fill_n : _Traits_ {
	template <class _Iterator_, class _DifferenceType_, class _ValueType_>
	struct __impl {
		_Iterator_ _iterator;
		_DifferenceType_ _left;
		const _ValueType_& _value;

		constexpr explicit __impl(_Iterator_ __it, _DifferenceType_ __size, const _ValueType_& __v) noexcept :
			_iterator(__it), _left(__size), _value(__v)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (!_left) return true;
			*_iterator++ = _value;
			_left--;
			return false;
		}

		constexpr raze_always_inline _Iterator_ result() const noexcept {
			return _iterator;
		}
	};

	template <class _Tag_>
	struct __vectorized_fill_n {
		template <class _Iterator_, class _DifferenceType_, class _ValueType_>
		raze_always_inline _Iterator_ operator()(_Iterator_ __first, _DifferenceType_ __size, const _ValueType_& __v)
			const noexcept
		{
			if constexpr (sizeof(_ValueType_) <= 8) {
				auto __address = std::to_address(__first);
				auto* __reinterpret_ptr = reinterpret_cast<_ValueType_*>(__address);

				for (; __size != 0; --__size, ++__reinterpret_ptr)
					*__reinterpret_ptr = __v;

				__seek_possibly_wrapped_iterator(__first, reinterpret_cast<std::iter_value_t<_Iterator_>*>(__reinterpret_ptr));
			}
			else {
				for (; __size != 0; --__size, ++__first)
					*__first = __v;
			}

			return __first;
		}

		template <class _Iterator_, class _DifferenceType_, class _ValueType_>
		raze_always_inline _Iterator_ operator()(sizetype __aligned_size, sizetype __tail_size,
			_Iterator_ __first, _DifferenceType_, const _ValueType_& __v) const noexcept requires(vx::simd_type<_Tag_>)
		{
			auto* __ptr = std::to_address(__first);
			raze_assume(__ptr != nullptr);

			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			do {
				vx::store(__ptr, _Tag_(__v));
				__advance_bytes(__ptr, sizeof(_Tag_));
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(__first, __ptr);
			return (*this)(__first, __tail_size / sizeof(_ValueType_), __v);
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _Iterator_, class _DifferenceType_, class _ValueType_>
		raze_always_inline _Iterator_ operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>, _Iterator_ __first, _DifferenceType_,
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
			return (*this)(__first, _TailSize_ / sizeof(_ValueType_), __v);
		}
	};

	template <class _Iterator_, class _Value_ = std::iter_value_t<_Iterator_>>
	constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first, 
		std::iter_difference_t<_Iterator_> __size, const std::type_identity_t<_Value_>& __v) const noexcept
			requires(std::output_iterator<_Iterator_, _Value_>)
	{
		__seek_possibly_wrapped_iterator(__first, __fill_n_unchecked(
			algorithm::__unwrap_iterator(std::move(__first)), __size, __v));
		return __first;
	}

	template <class _Iterator_, sizetype _Size_, class _Value_ = std::iter_value_t<_Iterator_>>
	constexpr raze_always_inline _Iterator_ operator()(_Iterator_ __first,
		std::integral_constant<sizetype, _Size_> __size, const std::type_identity_t<_Value_>& __v) const noexcept
		requires(std::output_iterator<_Iterator_, _Value_>)
	{
		__seek_possibly_wrapped_iterator(__first, __fill_n_unchecked(
			algorithm::__unwrap_iterator(std::move(__first)), __size, __v));
		return __first;
	}
private:
	template <class _Iterator_, class _DifferenceType_, class _ValueType_>
	constexpr raze_always_inline _Iterator_ __fill_n_unchecked(_Iterator_ __first,
		_DifferenceType_ __size, const _ValueType_& __v) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_Iterator_> && std::is_trivially_copyable_v<_Value_> &&
			sizeof(_Value_) <= 8 && (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				return vx::__dispatch_sized_impl<__vectorized_fill_n, _IntegerValue_, _Iterator_>(__size * sizeof(_Value_),
					__first, __size, math::bit_cast<_IntegerValue_>(__v));
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __size, __v));
	}

	template <class _Iterator_, sizetype _Size_, class _ValueType_>
	constexpr raze_always_inline _Iterator_ __fill_n_unchecked(_Iterator_ __first,
		std::integral_constant<sizetype, _Size_> __size, const _ValueType_& __v) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		if constexpr (std::contiguous_iterator<_Iterator_> && std::is_trivially_copyable_v<_Value_> &&
			sizeof(_Value_) <= 8 && (sizeof(_Value_) != 0) && (sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0)
		{
			if not consteval {
				using _IntegerValue_ = std::conditional_t<std::is_arithmetic_v<_Value_>, _Value_, typename IntegerForSizeof<_Value_>::Unsigned>;
				return vx::__dispatch_sized_impl<__vectorized_fill_n, _IntegerValue_, _Iterator_>(std::integral_constant<sizetype,
					__size * sizeof(_Value_)>{}, __first, __size, math::bit_cast<_IntegerValue_>(__v));
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __size, __v));
	}
};

constexpr inline auto fill_n = raze::options::function_with_traits<_Fill_n>;

__RAZE_ALGORITHM_NAMESPACE_END