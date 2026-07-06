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
struct _Copy_n : _Traits_ {
	template <class _InIterator_, class _OutIterator_>
	struct __impl {
		_InIterator_ _in_iterator;
		_OutIterator_ _out_iterator;
		std::iter_difference_t<_InIterator_> _n;

		constexpr explicit __impl(_InIterator_ __in_it, std::iter_difference_t<_InIterator_> __n, _OutIterator_ __out_it) noexcept :
			_in_iterator(__in_it), _out_iterator(__out_it), _n(__n)
		{}

		template <class _Tag_>
		raze_always_inline constexpr bool operator()(_Tag_) noexcept {
			if (_n <= 0) return true;
			*_out_iterator++ = *_in_iterator++;
			--_n;
			return false;
		}

		constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_> result() const noexcept {
			return { _in_iterator, _out_iterator };
		}
	};

	template <class _Tag_>
	struct __vectorized_copy_n {
		template <class _InIterator_, class _DifferenceType_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(_InIterator_ __first, _DifferenceType_ __n, _OutIterator_ __result) const noexcept {
			using _Type_ = std::iter_value_t<_InIterator_>;

			auto* __first_address = std::to_address(__first);
			auto* __result_address = std::to_address(__result);

			for (sizetype __i = 0; __i < __n; ++__i, ++__first_address, ++__result_address)
				*reinterpret_cast<_Type_*>(__result_address) = *reinterpret_cast<const _Type_* const>(__first_address);

			__seek_possibly_wrapped_iterator(__result, __result_address);
			__seek_possibly_wrapped_iterator(__first, __first_address);

			return { __first, __result };
		}

		template <class _InIterator_, class _DifferenceType_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(sizetype __aligned_size, sizetype __tail_size,
			_InIterator_ __first, _DifferenceType_, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
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

			return (*this)(__first, __tail_size / sizeof(typename _Tag_::value_type), __result);
		}

		template <sizetype _AlignedSize_, sizetype _TailSize_,
			class _InIterator_, class _DifferenceType_, class _OutIterator_>
		raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
		operator()(std::integral_constant<sizetype, _AlignedSize_>,
			std::integral_constant<sizetype, _TailSize_>,
			_InIterator_ __first, _DifferenceType_, _OutIterator_ __result) const noexcept requires(vx::simd_type<_Tag_>)
		{
			constexpr auto __iterations_aligned = _AlignedSize_ / sizeof(_Tag_);
			constexpr auto __tail_elems = _TailSize_ / sizeof(typename _Tag_::value_type);

			auto* __in_ptr = std::to_address(__first);
			auto* __out_ptr = std::to_address(__result);

			auto __left = __iterations_aligned;

			do {
				vx::store(__out_ptr, vx::load<_Tag_>(__in_ptr));
				__advance_bytes(__in_ptr, __out_ptr, sizeof(_Tag_));
			} while (--__left);

			__seek_possibly_wrapped_iterator(__first, __in_ptr);
			__seek_possibly_wrapped_iterator(__result, __out_ptr);

			return (*this)(__first, __tail_elems, __result);
		}
	};

	template <std::input_iterator _InIterator_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	operator()(_InIterator_ __first, std::iter_difference_t<_InIterator_> __n, _OutIterator_ __result) const noexcept
		requires(std::indirectly_copyable<_InIterator_, _OutIterator_>)
	{
		if (__n <= 0) return { std::move(__first), std::move(__result) };

		auto __r = __copy_n_unchecked(type_traits::__ranges_unwrap_iterator<_InIterator_>(std::move(__first)),
			static_cast<sizetype>(__n), algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

private:
	template <class _InIterator_, class _OutIterator_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	__copy_n_unchecked(_InIterator_ __first, sizetype __n, _OutIterator_ __result) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;
		using _IntegerValue_ = typename IntegerForSizeof<_Value_>::Unsigned;

		if constexpr (!options::always_scalar<_TraitsType>() && 
			std::contiguous_iterator<_InIterator_> && std::contiguous_iterator<_OutIterator_> &&
			std::is_trivially_copyable_v<_Value_> && sizeof(_Value_) <= 8 &&
			(sizeof(_Value_) != 0) && ((sizeof(_Value_) & (sizeof(_Value_) - 1)) == 0))
		{
			if not consteval {
				return vx::__dispatch_sized_impl<__vectorized_copy_n, _IntegerValue_,
					std::ranges::in_out_result<_InIterator_, _OutIterator_>, options::__get_forced_isa<_TraitsType>()>(
					__n * sizeof(_Value_), __first, __n, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __n, __result));
	}
};

constexpr inline auto copy_n = raze::options::function_with_traits<_Copy_n>;

__RAZE_ALGORITHM_NAMESPACE_END
