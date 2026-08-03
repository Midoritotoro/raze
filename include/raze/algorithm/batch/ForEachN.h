#pragma once 


#include <src/raze/algorithm/RangesSize.h>
#include <src/raze/algorithm/VectorizablePredicate.h>
#include <src/raze/algorithm/EqualTo.h>
#include <src/raze/algorithm/NotFn.h>
#include <src/raze/vx/dispatch/SizedSimdDispatcher.h>
#include <raze/options/Options.h>
#include <algorithm>

__RAZE_ALGORITHM_NAMESPACE_BEGIN

template <class _Traits_>
struct _For_each_n : _Traits_ {
	template <class _Iterator_, class _Function_, class _Projection_>
	struct __impl {
		mutable _Iterator_ _iterator;
		_Function_ _function;
		_Projection_ _proj;
		sizetype _count;

		constexpr explicit __impl(_Iterator_ __it, sizetype __n, _Function_ __f, _Projection_ __proj) noexcept :
			_iterator(__it), _function(__f), _proj(__proj), _count(__n)
		{}

		template <class _Tag_>
		raze_always_inline constexpr void operator()(_Tag_) const noexcept
			requires(options::concepts::same_as<_Tag_, vx::scalar_tag>)
		{
			raze_disable_unrolling
			for (sizetype __i = 0; __i < _count; ++__i, ++_iterator)
				_function(_proj(*_iterator));
		}

		template <class _Tag_>
		raze_always_inline constexpr void operator()(_Tag_, sizetype __aligned_size) const noexcept
			requires(!options::concepts::same_as<_Tag_, vx::scalar_tag>)
		{
			auto* __ptr = std::to_address(_iterator);
			const auto __aligned_end = __bytes_pointer_offset(__ptr, __aligned_size);

			raze_disable_unrolling
			do {
				auto __projected = _proj(raze::vx::load<_Tag_>(__ptr));
				_function(__projected);
				vx::store(__ptr, __projected);
				__ptr += _Tag_::size();
			} while (__ptr != __aligned_end);

			__seek_possibly_wrapped_iterator(_iterator, __ptr);
		}

		constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> result() const noexcept {
			return { _iterator, _function };
		}
	};

	template <std::input_iterator _Iterator_, class _Function_, class _Projection_ = std::identity>
	constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> operator()(_Iterator_ __first,
		sizetype __n, _Function_ __f, _Projection_ __proj = {}) const noexcept
	{
		if (__n == 0) return { std::move(__first), std::move(__f) };

		auto __r = __for_each_n_unchecked(
			type_traits::__ranges_unwrap_iterator<_Iterator_>(std::move(__first)),
			__n, type_traits::__pass_function(__f), type_traits::__pass_function(__proj));

		__seek_possibly_wrapped_iterator(__first, __r.in);
		return { __first, __unwrap_function(std::move(__r.fun)) };
	}

private:
	template <class _Iterator_, class _Function_, class _Projection_>
	constexpr raze_always_inline std::ranges::for_each_result<_Iterator_, _Function_> __for_each_n_unchecked(
		_Iterator_ __first, sizetype __n, _Function_ __f, _Projection_ __proj) const noexcept
	{
		using _TraitsType = decltype(this->traits());
		using _Value_ = std::iter_value_t<_Iterator_>;

		auto __work = __impl(__first, __n, __f, __proj);

		if constexpr (!options::always_scalar<_TraitsType>() && std::contiguous_iterator<_Iterator_> 
			&& vectorizable_unary_function<_Function_, _Iterator_> &&
			vectorizable_projection<_Projection_, _Iterator_> && type_traits::__is_lightweight_callable_v<_Function_>)
		{
			if not consteval {
				return vx::__dispatch_sized_impl<options::_Unroller<_TraitsType>::template __impl, 
					_Value_, std::ranges::for_each_result<_Iterator_, _Function_>>(__n * sizeof(_Value_), __work);
			}
		}
		
		return options::__unroller<decltype(this->traits()), vx::scalar_tag>(__work);
	}
};

constexpr inline auto for_each_n = raze::options::function_with_traits<_For_each_n>[raze::options::unroll<4>];

__RAZE_ALGORITHM_NAMESPACE_END