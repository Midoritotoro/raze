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

	template <std::input_iterator _InIterator_, std::weakly_incrementable _OutIterator_>
	constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	operator()(_InIterator_ __first, std::iter_difference_t<_InIterator_> __n, _OutIterator_ __result) const noexcept
		requires(std::indirectly_copyable<_InIterator_, _OutIterator_>)
	{
		auto __r = __copy_n_unchecked(type_traits::__ranges_unwrap_iterator<_InIterator_>(std::move(__first)),
			static_cast<sizetype>(__n), algorithm::__unwrap_iterator(std::move(__result)));

		__seek_possibly_wrapped_iterator(__first, std::move(__r.in));
		__seek_possibly_wrapped_iterator(__result, std::move(__r.out));

		return { std::move(__first), std::move(__result) };
	}

private:
	template <class _InIterator_, class _OutIterator_>
	raze_nodiscard constexpr raze_always_inline std::ranges::in_out_result<_InIterator_, _OutIterator_>
	__copy_n_unchecked(_InIterator_ __first, std::iter_difference_t<_InIterator_> __n, _OutIterator_ __result) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_InIterator_>;
		using _IntegerValue_ = typename IntegerForSizeof<_Value_>::Unsigned;

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_InIterator_> && 
			std::contiguous_iterator<_OutIterator_> && std::is_trivially_copyable_v<_Value_>)
		{
			if not consteval {
				auto* __first_ptr = std::to_address(__first);
				auto* __r_ptr = std::to_address(__result);

				auto __e = __raze_memcpy(__r_ptr, __first_ptr, __n * sizeof(_Value_));

				__seek_possibly_wrapped_iterator(__first, __first_ptr + __n);
				__seek_possibly_wrapped_iterator(__result, static_cast<decltype(__r_ptr)>(__e));

				return std::ranges::in_out_result(__first, __result);
			}
		}

		return options::__unroller<_TraitsType, vx::scalar_tag>(__impl(__first, __n, __result));
	}
};

constexpr inline auto copy_n = raze::options::function_with_traits<_Copy_n>;

__RAZE_ALGORITHM_NAMESPACE_END
